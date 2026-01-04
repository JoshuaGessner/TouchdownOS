/**
 * @file dbus_interface.hpp
 * @brief D-Bus interface base class for TouchdownOS services
 */

#ifndef TOUCHDOWN_SERVICES_DBUS_INTERFACE_HPP
#define TOUCHDOWN_SERVICES_DBUS_INTERFACE_HPP

#include <dbus/dbus.h>
#include <string>
#include <memory>
#include <functional>
#include <map>

namespace touchdown {
namespace services {

class DBusInterface {
public:
    DBusInterface(const std::string& service_name, const std::string& object_path);
    virtual ~DBusInterface();
    
    /**
     * @brief Initialize D-Bus connection
     */
    bool init();
    
    /**
     * @brief Process D-Bus messages (call in main loop)
     */
    void process();
    
    /**
     * @brief Send signal
     */
    void send_signal(const std::string& interface, const std::string& name, 
                     const std::string& arg = "");
    
    /**
     * @brief Notify systemd of readiness
     */
    void notify_ready();
    
    /**
     * @brief Send watchdog keepalive
     */
    void send_watchdog();
    
protected:
    using MethodHandler = std::function<DBusMessage*(DBusMessage*)>;
    
    /**
     * @brief Register a method handler
     */
    void register_method(const std::string& interface, const std::string& method, 
                        MethodHandler handler);
    
    /**
     * @brief Handle incoming method calls
     */
    virtual DBusMessage* handle_message(DBusMessage* msg);
    
    std::string service_name_;
    std::string object_path_;
    DBusConnection* connection_;
    
private:
    struct MethodKey {
        std::string interface;
        std::string method;
        
        bool operator<(const MethodKey& other) const {
            return std::tie(interface, method) < std::tie(other.interface, other.method);
        }
    };
    
    std::map<MethodKey, MethodHandler> method_handlers_;
    
    static DBusHandlerResult message_handler(DBusConnection* connection, 
                                            DBusMessage* message, void* user_data);
};

} // namespace services
} // namespace touchdown

#endif // TOUCHDOWN_SERVICES_DBUS_INTERFACE_HPP
