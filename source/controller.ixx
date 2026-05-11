module;

#include <common.hxx>

export module Controller;

import common;

export namespace CFrontEnd
{
    bool(__cdecl* CheckInput)(int a1, char a2, char a3, char a4, char a5, char a6, char a7) = nullptr;
    bool(__cdecl* iscontrollerFrontendMode)() = nullptr;
    uintptr_t(__cdecl* GetFrontEndData)() = nullptr;
    uint8_t* flags = nullptr;
    bool IsLastInputMouse()
    {
        auto data = GetFrontEndData();
        return *(bool*)(data + 0x328D);
    }
    bool IsLastInputController()
    {
        auto data = GetFrontEndData();
        return *(bool*)(data + 0x328C);
    }
}
static uintptr_t MenuLeft_address;
static uintptr_t MenuRight_address;
void OpenConsole()
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    printf("Oh,Hi Mark\n");
}
class Controller
{
public:

    Controller()
    {
        FusionFix::onInitEvent() += []()
        {
                OpenConsole();
                auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 0F 84 ? ? ? ? 8B 43 ? 83 F8 ? 0F 87 ? ? ? ? 0F B6 80 ? ? ? ? FF 24 85 ? ? ? ? E8");
                if (!pattern.empty())
                {
                     CFrontEnd::CheckInput = (decltype(CFrontEnd::CheckInput))injector::GetBranchDestination(pattern.get_first()).get();
                }

                pattern = hook::pattern("E8 ? ? ? ? 8B 35 ? ? ? ? 84 C0 75");
                if (!pattern.empty())
                {
                    CFrontEnd::iscontrollerFrontendMode = (decltype(CFrontEnd::iscontrollerFrontendMode))injector::GetBranchDestination(pattern.get_first()).get();
                }

                pattern = hook::pattern("F6 05 ? ? ? ? ? 6A 00");
                if (!pattern.empty())
                {
                    CFrontEnd::flags = ((*pattern.get_first<uint8_t*>(2)));
                }

                pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 8B C8 E8 ? ? ? ? C6 05 ? ? ? ? 00 5B");
                if (!pattern.empty())
                {
                    CFrontEnd::GetFrontEndData = (decltype(CFrontEnd::GetFrontEndData))injector::GetBranchDestination(pattern.get_first()).get();
                }

                pattern = hook::pattern("8B 43 ? 83 F8 ? 0F 87 ? ? ? ? 0F B6 80 ? ? ? ? FF 24 85 ? ? ? ? E8");
                if (!pattern.empty())
                {
                    MenuLeft_address = (uintptr_t)pattern.get_first();
                }
                pattern = hook::pattern("8B 43 ? 83 F8 ? 0F 87 ? ? ? ? 0F B6 80 ? ? ? ? FF 24 85 ? ? ? ? C6 05 ? ? ? ? 00");

                if (!pattern.empty())
                {
                    MenuRight_address = (uintptr_t)pattern.get_first();
                }

                if (MenuRight_address && MenuLeft_address)
                {
                    pattern = hook::pattern("0F 85 ? ? ? ? 6A 00 6A 00 55");
                    if (!pattern.empty())
                    {
                        static auto JumpBetweenMenus = safetyhook::create_mid(pattern.get_first(), [](SafetyHookContext& regs) {
                            if (CFrontEnd::flags) {
                                const uint8_t flags = *CFrontEnd::flags;

                                printf("flags = 0x%02X | set bits:", flags);
                                for (uint8_t bit = 1; bit != 0; bit <<= 1) {
                                    if (flags & bit)
                                        printf(" 0x%02X", bit);
                                }
                                if (flags == 0)
                                    printf(" none");
                                printf("\n");
                                int LB = 19;
                                int RB = 20;
                                auto is_mouse = CFrontEnd::IsLastInputMouse();
                                if (is_mouse)
                                {
                                    LB = 20;
                                    RB = 19;
                                }
                                if (is_mouse && (flags & 0x30) != 0)
                                    return;

                                if (CFrontEnd::CheckInput(LB, 1, 0, 0, 1, 0, 0))
                                    regs.eip = MenuLeft_address;
                                else if (CFrontEnd::CheckInput(RB, 1, 0, 0, 1, 0, 0))
                                    regs.eip = MenuRight_address;
                            }
                            });

                    }
                }

        };
    }
} Controller;