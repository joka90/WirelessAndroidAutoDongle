#include <dirent.h> // readdir
#include <fcntl.h> // open
#include <sys/stat.h> // stat
#include <unistd.h> // unlinkat symlinkat
#include <string.h>
#include <future>

#include "common.h"
#include "uevent.h"
#include "usb.h"

constexpr const char* defaultGadgetName = "default";
constexpr const char* accessoryGadgetName = "accessory";

/*static*/ bool UsbManager::s_have_adb;
/*static*/ std::string UsbManager::s_udcName;

UsbManager& UsbManager::instance() {
    static UsbManager instance;
    return instance;
}

void UsbManager::init() {
    // Init does not actually do anything but provides an intuitive place to cause the constructor call earlier than first use.
}

UsbManager::UsbManager() {
    Logger::instance()->info("Initializing USB Manager\n");
    struct stat statbuf;

    s_have_adb = true;
    if (stat("/dev/android_adb", &statbuf) == -1) {
        s_have_adb = false;
    }

    DIR* dirSysClassUdc = opendir("/sys/class/udc/");
    if (dirSysClassUdc == NULL) {
        Logger::instance()->info("USB Manager: Error opening /sys/class/udc/: %s\n", strerror(errno));
        return;
    }
    
    struct dirent* dirEntry = NULL;
    while ((dirEntry = readdir(dirSysClassUdc)) != NULL) {
        if (dirEntry->d_name[0] == '.') {
            continue;
        }

        s_udcName = dirEntry->d_name;
    }

    closedir(dirSysClassUdc);

    if (s_udcName.empty()) {
        Logger::instance()->info("USB Manager: Did not find a valid UDC to use\n");
    } else {
        Logger::instance()->info("USB Manager: Found UDC %s\n", s_udcName.c_str());
    }

    // if we are restarted, reset state
    disableGadget();
}


static void unlinkFuncs(std::string path, std::string relativeFilePath) {
    std::string dir = path + "/" + relativeFilePath;
    DIR* dirUnlink = opendir(dir.c_str());
    int dir_fd = dirfd(dirUnlink); // It will be automatically closed when closedir(3) is
    if (dirUnlink == NULL) {
        Logger::instance()->info("USB Manager: Error opening %s: %s\n", dir.c_str(), strerror(errno));
        return;
    }

    struct dirent* dirEntry = NULL;
    while ((dirEntry = readdir(dirUnlink)) != NULL) {
        // match all f[0-9] that are symlinks
        if (dirEntry->d_name[0] != 'f' || dirEntry->d_type != DT_LNK) {
            continue;
        }
        if ('0' <= dirEntry->d_name[1] && '9' >= dirEntry->d_name[1] && dirEntry->d_name[2] == 0) {
            if(unlinkat(dir_fd, dirEntry->d_name, 0)) {
                Logger::instance()->info("USB Manager: Error unlinkat %s %s: %s\n", dir.c_str(), dirEntry->d_name, strerror(errno));
            }
        }
    }

    closedir(dirUnlink);
}

static void writeFile(std::string path, std::string relativeFilePath, const char* content) {
    std::string filePath = path + "/" + relativeFilePath;
    FILE* f = fopen(filePath.c_str(), "w");
    fputs(content, f);
    fputc('\n', f);
    fclose(f);
}

static void linkFunc(const char* func_path, int dir_fd, const char* config_path) {
    if (symlinkat(func_path, dir_fd, config_path) == -1) {
        Logger::instance()->info("USB Manager: failed to link %s -> %s : %s\n", func_path, config_path, strerror(errno));
    }
}

void UsbManager::enableGadget(gadget gadgetName) {
    std::string gadgetFilePath = "/sys/kernel/config/usb_gadget/g1";
    writeFile(gadgetFilePath, "UDC", "");
    unlinkFuncs(gadgetFilePath, "configs/c.1");
    if (gadgetName == none && !s_have_adb) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 0.1 second, keep the gadget disabled for a short time to let the host recognize the change

    int dir_fd = open(gadgetFilePath.c_str(), O_RDONLY | O_PATH | O_DIRECTORY);
    if (dir_fd == -1) {
        Logger::instance()->info("USB Manager: Error opening %s: %s\n", gadgetFilePath.c_str(), strerror(errno));
        return;
    }

    if (gadgetName == none && s_have_adb) {
        writeFile(gadgetFilePath, "idProduct", "0x4ee7");
        linkFunc("functions/ffs.adb", dir_fd, "configs/c.1/f1");
    } else if (gadgetName == mtp && !s_have_adb) {
        writeFile(gadgetFilePath, "idProduct", "0x4ee1");
        writeFile(gadgetFilePath, "strings/0x409/configuration", "mtp");
        linkFunc("functions/ffs.mtp", dir_fd, "configs/c.1/f1");
    } else if (gadgetName == mtp && s_have_adb) {
        writeFile(gadgetFilePath, "idProduct", "0x4ee2");
        writeFile(gadgetFilePath, "strings/0x409/configuration", "mtp_adb");
        linkFunc("functions/ffs.mtp", dir_fd, "configs/c.1/f1");
        linkFunc("functions/ffs.adb", dir_fd, "configs/c.1/f2");
    } else if (gadgetName == accessory && !s_have_adb) {
        writeFile(gadgetFilePath, "idProduct", "0x2d00");
        writeFile(gadgetFilePath, "strings/0x409/configuration", "accessory");
        linkFunc("functions/accessory.usb0", dir_fd, "configs/c.1/f1");
    } else if (gadgetName == accessory && s_have_adb) {
        writeFile(gadgetFilePath, "idProduct", "0x2d01");
        writeFile(gadgetFilePath, "strings/0x409/configuration", "accessory_adb");
        linkFunc("functions/accessory.usb0", dir_fd, "configs/c.1/f1");
        linkFunc("functions/ffs.adb", dir_fd, "configs/c.1/f2");
    }
    close(dir_fd);

    writeFile(gadgetFilePath, "UDC", s_udcName.c_str());
}

// void UsbManager::disableGadget(std::string gadgetName) {
//     writeGadgetFile(gadgetName, "UDC", "");
// }

void UsbManager::switchToAccessoryGadget() {
    enableGadget(accessory);

    Logger::instance()->info("USB Manager: Switched to accessory gadget from default\n");
}

void UsbManager::disableGadget() {
    enableGadget(none);

    Logger::instance()->info("USB Manager: Disabled all USB gadgets\n");
}

bool UsbManager::enableDefaultAndWaitForAccessory(std::chrono::milliseconds timeout) {
    std::shared_ptr<std::promise<void>> accessoryPromise = std::make_shared<std::promise<void>>();
    std::weak_ptr<std::promise<void>> accessoryPromiseWeak = accessoryPromise;

    UeventMonitor::instance().addHandler([accessoryPromiseWeak](UeventEnv env) {
        std::shared_ptr<std::promise<void>> accessoryPromise = accessoryPromiseWeak.lock();

        // If the promise is no longer active, nothing to do.
        if (!accessoryPromise) {
            return true;
        }

        if (auto it = env.find("DEVNAME"); it == env.end() || it->second != "usb_accessory") {
            return false;
        }

        if (auto it = env.find("ACCESSORY"); it == env.end() || it->second != "START") {
            return false;
        }

        // Got an accessory start event
        Logger::instance()->info("USB Manager: Received accessory start request\n");
        UsbManager::instance().switchToAccessoryGadget();
        accessoryPromise->set_value();

        return true;
    });

    enableGadget(mtp);

    Logger::instance()->info("USB Manager: Enabled default gadget\n");

    if (timeout == std::chrono::milliseconds(0)) {
        accessoryPromise->get_future().wait();
        return true;
    } else {
        std::future_status status = accessoryPromise->get_future().wait_for(timeout);

        if (status == std::future_status::ready) {
            return true;
        } else {
            Logger::instance()->info("USB Manager: Timeout waiting for accessory start request\n");
            return false;
        }
    }
}
