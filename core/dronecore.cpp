#include "dronecore.h"
#include "dronecore_impl.h"
#include "global_include.h"
#include "connection.h"
#include "udp_connection.h"
#include "tcp_connection.h"
#ifndef WINDOWS
#include "serial_connection.h"
#endif


namespace dronecore {

DroneCore::DroneCore() :
    _impl(nullptr)
{
    _impl = new DroneCoreImpl();
}

DroneCore::~DroneCore()
{
    delete _impl;
    _impl = nullptr;
}

DroneCore::ConnectionResult DroneCore::add_any_connection(std::string str)
{
    std::string delimiter = "://";
    std::string connection_str[3];
    size_t pos = 0, i = 0;
    while(i<2)
    {
        pos = str.find(delimiter);
        if(pos != std::string::npos)
        {
            connection_str[i] = str.substr(0, pos);
            str.erase(0, pos+delimiter.length());
            if(str == "")
				break;
            delimiter=":";
        }
	    i++;
    }
    connection_str[2] = str;
    if(connection_str[0] != "serial")
        return add_link_connection(connection_str);
    else if(connection_str[0] == "serial")
    {
        if(connection_str[1] == "")
            return add_serial_connection();
        else
            return add_serial_connection(connection_str[1], stoi(connection_str[2]));
    }
	return DroneCore::ConnectionResult::DESTINATION_IP_UNKNOWN;
}


DroneCore::ConnectionResult DroneCore::add_link_connection(std::string* connection_str)
{
    int local_port_number = 0;
    if(connection_str[2] == "")
    {
        if(connection_str[1] != "")
        {
            local_port_number = stoi(connection_str[1]);
            /* default ip for tcp if only port number is specified */
            connection_str[1]="127.0.0.1";
        }
    }
    else
    {
        local_port_number = stoi(connection_str[2]);
    }
    if(connection_str[0] == "udp")
    {
        if(local_port_number == 0)
            return add_udp_connection();
        else
        {
            return add_udp_connection(local_port_number);
        }
    }
    if(connection_str[0] == "tcp")
    {
        if(local_port_number == 0)
            return add_tcp_connection();
        else
        {
            return add_tcp_connection(connection_str[1], local_port_number);
        }
    }
	return DroneCore::ConnectionResult::DESTINATION_IP_UNKNOWN;

}

DroneCore::ConnectionResult DroneCore::add_udp_connection()
{
    return add_udp_connection(0);
}

DroneCore::ConnectionResult DroneCore::add_udp_connection(int local_port_number)
{
    Connection *new_connection = new UdpConnection(_impl, local_port_number);
    DroneCore::ConnectionResult ret = new_connection->start();

    if (ret != DroneCore::ConnectionResult::SUCCESS) {
        delete new_connection;
        return ret;
    }

    _impl->add_connection(new_connection);
    return DroneCore::ConnectionResult::SUCCESS;
}

DroneCore::ConnectionResult DroneCore::add_tcp_connection()
{
    return add_tcp_connection("", 0);
}

DroneCore::ConnectionResult DroneCore::add_tcp_connection(std::string remote_ip,
                                                          int remote_port)
{
    Connection *new_connection = new TcpConnection(_impl, remote_ip, remote_port);
    DroneCore::ConnectionResult ret = new_connection->start();

    if (ret != DroneCore::ConnectionResult::SUCCESS) {
        delete new_connection;
        return ret;
    }

    _impl->add_connection(new_connection);
    return DroneCore::ConnectionResult::SUCCESS;
}

DroneCore::ConnectionResult DroneCore::add_serial_connection()
{
#if !defined(WINDOWS) && !defined(APPLE)
    return add_serial_connection("", 0);
#else
    return ConnectionResult::NOT_IMPLEMENTED;
#endif
}

DroneCore::ConnectionResult DroneCore::add_serial_connection(std::string dev_path,
                                                             int baudrate)
{
#if !defined(WINDOWS) && !defined(APPLE)
    Connection *new_connection = new SerialConnection(_impl, dev_path, baudrate);
    DroneCore::ConnectionResult ret = new_connection->start();

    if (ret != DroneCore::ConnectionResult::SUCCESS) {
        delete new_connection;
        return ret;
    }

    _impl->add_connection(new_connection);
    return DroneCore::ConnectionResult::SUCCESS;
#else
    UNUSED(dev_path);
    UNUSED(baudrate);
    return DroneCore::ConnectionResult::NOT_IMPLEMENTED;
#endif
}

const std::vector<uint64_t> &DroneCore::device_uuids() const
{
    return _impl->get_device_uuids();
}

Device &DroneCore::device() const
{
    return _impl->get_device();
}

Device &DroneCore::device(uint64_t uuid) const
{
    return _impl->get_device(uuid);
}

bool DroneCore::is_connected() const
{
    return _impl->is_connected();
}

bool DroneCore::is_connected(uint64_t uuid) const
{
    return _impl->is_connected(uuid);
}

void DroneCore::register_on_discover(event_callback_t callback)
{
    _impl->register_on_discover(callback);
}

void DroneCore::register_on_timeout(event_callback_t callback)
{
    _impl->register_on_timeout(callback);
}

const char *DroneCore::connection_result_str(ConnectionResult result)
{
    switch (result) {
        case ConnectionResult::SUCCESS:
            return "Success";
        case ConnectionResult::TIMEOUT:
            return "Timeout";
        case ConnectionResult::SOCKET_ERROR:
            return "Socket error";
        case ConnectionResult::BIND_ERROR:
            return "Bind error";
        case ConnectionResult::SOCKET_CONNECTION_ERROR:
            return "Socket connection error";
        case ConnectionResult::CONNECTION_ERROR:
            return "Connection error";
        case ConnectionResult::NOT_IMPLEMENTED:
            return "Not implemented";
        case ConnectionResult::DEVICE_NOT_CONNECTED:
            return "Device not connected";
        case ConnectionResult::DEVICE_BUSY:
            return "Device busy";
        case ConnectionResult::COMMAND_DENIED:
            return "Command denied";
        case ConnectionResult::DESTINATION_IP_UNKNOWN:
            return "Destination IP unknown";
        case ConnectionResult::CONNECTIONS_EXHAUSTED:
            return "Connections exhausted";
        default:
            return "Unknown";
    }
}

} // namespace dronecore
