#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define GSETTINGS_SCHEMA_PROXY  "org.gnome.system.proxy"
#define GSETTINGS_SCHEMA_HTTP   "org.gnome.system.proxy.http"
#define GSETTINGS_SCHEMA_HTTPS  "org.gnome.system.proxy.https"
#define GSETTINGS_SCHEMA_FTP    "org.gnome.system.proxy.ftp"
#define GSETTINGS_SCHEMA_SOCKS  "org.gnome.system.proxy.socks"

#define GSETTINGS_KEY_USE_SAME_PROXY  "use-same-proxy"
#define GSETTINGS_KEY_AUTOCONFIG_URL  "autoconfig-url"
#define GSETTINGS_KEY_IGNORE_HOSTS    "ignore-hosts"
#define GSETTINGS_KEY_MODE  "mode"
#define GSETTINGS_KEY_HOST  "host"
#define GSETTINGS_KEY_PORT  "port"
#define GSETTINGS_KEY_USE_AUTH    "use-authentication"
#define GSETTINGS_KEY_AUTH_USER   "authentication-user"
#define GSETTINGS_KEY_AUTH_PASS   "authentication-password"

#define GSETTINGS_MODE_NONE   "none"
#define GSETTINGS_MODE_AUTO   "auto"
#define GSETTINGS_MODE_MANUAL "manual"

#define BLOCK_BEGIN "#BEGIN_BLOCK\n"
#define BLOCK_END "#END_BLOCK\n"
#define BLOCK_COMMENT "#DO NOT EDIT THIS BLOCK BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"

#endif // MAIN_H_INCLUDED
