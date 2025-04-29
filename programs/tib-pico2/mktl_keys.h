//
// Created by Jeb Bailey on 4/25/25.
//

#ifndef KEYS_H
#define KEYS_H
#include <string_view>

namespace mktl_keys {
    // ====================
    // Attenuator Keys
    // ====================
    inline constexpr std::string_view ATT_Y_1028_PREFIX  = "1028_atten.";
    inline constexpr std::string_view ATT_J_1270_PREFIX  = "1270_atten.";
    inline constexpr std::string_view ATT_YJ_1410_PREFIX = "yj1410_atten.";
    inline constexpr std::string_view ATT_HK_1410_PREFIX = "hk1410_atten.";
    inline constexpr std::string_view ATT_H_1510_PREFIX  = "1510_atten.";
    inline constexpr std::string_view ATT_K_2330_PREFIX  = "2330_atten.";
    inline constexpr std::string_view ATT_DB_SUFFIX      = "db";
    inline constexpr std::string_view ATT_CALIB_SUFFIX   = "calibration";

    // ====================
    // Photodiode Keys
    // ====================
    inline constexpr std::string_view PD_YJ_PREFIX        = "yj_diode.";
    inline constexpr std::string_view PD_HK_PREFIX        = "hk_diode.";
    inline constexpr std::string_view PD_VALUE_SUFFIX     = "value";

    // ====================
    // MEMS Switch Keys
    // ====================
    inline constexpr std::string_view MEMS_SWITCH         = "mems.switch";
    inline constexpr std::string_view MEMS_ROUTE          = "mems.route";

    // ====================
    // Laser Diode Keys
    // ====================
    inline constexpr std::string_view LASER_Y_1028_PREFIX  = "1028_laser.";
    inline constexpr std::string_view LASER_J_1270_PREFIX  = "1270_laser.";
    inline constexpr std::string_view LASER_YJ_1410_PREFIX = "yj1420_laser.";
    inline constexpr std::string_view LASER_HK_1410_PREFIX = "hk1420_laser.";
    inline constexpr std::string_view LASER_H_1510_PREFIX  = "1510_laser.";
    inline constexpr std::string_view LASER_K_2330_PREFIX  = "2330_laser.";
    inline constexpr std::string_view LASER_LEVEL_SUFFIX   = "level";    // 0 = off
    inline constexpr std::string_view LASER_STATUS_SUFFIX  = "status";
    inline constexpr std::string_view LASER_CONFIG_SUFFIX  = "config";

    // ====================
    // General System Keys
    // ====================
    inline constexpr std::string_view SYSTEM_REBOOT       = "system.reboot";
    inline constexpr std::string_view SYSTEM_VERSION      = "system.version";
    inline constexpr std::string_view SYSTEM_STATUS       = "system.status";
    inline constexpr std::string_view SYSTEM_POWER        = "system.power";

} // namespace
#endif //KEYS_H

