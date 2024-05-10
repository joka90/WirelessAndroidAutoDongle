#include <string>
#include <chrono>

class UsbManager {
public:
    static UsbManager& instance();

    void init();
    bool enableDefaultAndWaitForAccessory(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    void switchToAccessoryGadget();
    void disableGadget();

private:
    enum gadget : char { none = 0 , accessory = 1, mtp = 2 };

    UsbManager();
    UsbManager(UsbManager const&);
    UsbManager& operator=(UsbManager const&);

    void enableGadget(gadget name);

    static bool s_have_adb;
    static std::string s_udcName; 
};
