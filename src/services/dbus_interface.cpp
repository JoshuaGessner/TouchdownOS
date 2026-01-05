/**
 * @file dbus_interface.cpp
 * @brief D-Bus interface implementation
 */

#include "touchdown/services/dbus_interface.hpp"
#include "touchdown/core/logger.hpp"
#include <systemd/sd-daemon.h>
#include <cstring>

namespace touchdown {
namespace services {

DBusInterface::DBusInterface(const std::string& service_name, const std::string& object_path)
    : service_name_(service_name)
    , object_path_(object_path)
    , connection_(nullptr) {
}

DBusInterface::~DBusInterface() {
    if (connection_) {
        dbus_connection_unref(connection_);
    }
}

bool DBusInterface::init() {
    DBusError error;
    dbus_error_init(&error);
    
    // Connect to system bus
    connection_ = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        TD_LOG_ERROR("DBusInterface", "Failed to connect to D-Bus: ", error.message);
        dbus_error_free(&error);
        return false;
    }
    
    // Request service name
    int ret = dbus_bus_request_name(connection_, service_name_.c_str(),
                                     DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    
    if (dbus_error_is_set(&error)) {
        TD_LOG_ERROR("DBusInterface", "Failed to request name: ", error.message);
        dbus_error_free(&error);
        return false;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        TD_LOG_ERROR("DBusInterface", "Not primary owner of name: ", service_name_);
        return false;
    }
    
    // Add message filter
    dbus_connection_add_filter(connection_, message_handler, this, nullptr);
    
    TD_LOG_INFO("DBusInterface", "D-Bus service initialized: ", service_name_);
    return true;
}

void DBusInterface::process() {
    if (!connection_) return;
    
    // Process pending messages
    while (dbus_connection_dispatch(connection_) == DBUS_DISPATCH_DATA_REMAINS) {
        // Continue processing
    }
    
    // Read messages without blocking
    dbus_connection_read_write(connection_, 0);
}

void DBusInterface::send_signal(const std::string& interface, const std::string& name, 
                                const std::string& arg) {
    if (!connection_) return;
    
    DBusMessage* msg = dbus_message_new_signal(object_path_.c_str(), 
                                               interface.c_str(), 
                                               name.c_str());
    if (!msg) {
        TD_LOG_ERROR("DBusInterface", "Failed to create signal message");
        return;
    }
    
    if (!arg.empty()) {
        const char* str = arg.c_str();
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
    }
    
    dbus_connection_send(connection_, msg, nullptr);
    dbus_connection_flush(connection_);
    dbus_message_unref(msg);
}

void DBusInterface::notify_ready() {
    sd_notify(0, "READY=1");
    TD_LOG_INFO("DBusInterface", "Notified systemd: READY");
}

void DBusInterface::send_watchdog() {
    sd_notify(0, "WATCHDOG=1");
}

void DBusInterface::register_method(const std::string& interface, const std::string& method,
                                   MethodHandler handler) {
    MethodKey key{interface, method};
    method_handlers_[key] = handler;
}

DBusMessage* DBusInterface::handle_message(DBusMessage* msg) {
    const char* interface = dbus_message_get_interface(msg);
    const char* member = dbus_message_get_member(msg);
    
    if (!interface || !member) return nullptr;
    
    MethodKey key{interface, member};
    auto it = method_handlers_.find(key);
    
    if (it != method_handlers_.end()) {
        return it->second(msg);
    }
    
    return nullptr;
}

DBusHandlerResult DBusInterface::message_handler(DBusConnection* connection,
                                                DBusMessage* message, void* user_data) {
    DBusInterface* self = static_cast<DBusInterface*>(user_data);
    
    if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
        DBusMessage* reply = self->handle_message(message);
        
        if (reply) {
            dbus_connection_send(connection, reply, nullptr);
            dbus_message_unref(reply);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

} // namespace services
} // namespace touchdown
