#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


using namespace std;
const int RES_SUCCESS = -1;
const int RES_FAILED = 0;
 
const char *const SERVER_BUS_NAME = "fm.name.bus";
const char *const INTERFACE_NAME = "fm.name.interface";
const char *const OBJECTPATH = "/fm/name/object/path";

const char *const SIGNAL_NAME = "signal";
 
static DBusHandlerResult method_message(DBusConnection *connection, DBusMessage *request);
static void respond_to_introspect(DBusConnection *connection, DBusMessage *request);
static void respond_to_Signal(DBusConnection *connection, DBusMessage* msg);

static void reply_method_call_add(DBusConnection *conn, DBusMessage *msg);

static void reply_method_set_value(DBusConnection *conn, DBusMessage *msg);
static void reply_method_get_value(DBusConnection *conn, DBusMessage *msg);

int my_dbus_initialization(char const *_bus_name, DBusConnection **_conn)
{
    DBusError err;
    int ret;
 
    dbus_error_init(&err);
 
    *_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err))
    {
        cout<<"Connection Error:"<<err.message<<endl;
        dbus_error_free(&err);
        return RES_FAILED;
    }
 
    ret = dbus_bus_request_name(*_conn, _bus_name,
                                DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err))
    {
        //cout<<"Replace error"<<err.message<<endl;
        dbus_error_free(&err);
        return RES_FAILED;
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
    {
        return RES_FAILED;
    }
    return RES_SUCCESS;
}
 
static void respond_to_introspect(DBusConnection *connection, DBusMessage *request)
{
    //cout<<"called introspect"<<endl;
	DBusMessage *reply;
	const char *introspection_data =
		"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \n"
		"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
		"<node>\n"
		"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
    		"    <method name=\"Introspect\">\n"
      		"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
    		"    </method>\n"
  		"  </interface>\n"
		"  <interface name=\"fm.name.interface\">\n"
        "<method name=\"Add\">\
            <arg name=\"arg0\" type=\"i\"/>\
            <arg name=\"arg1\" type=\"i\"/>\
            <arg name=\"ret\" type=\"i\" direction=\"out\"/>\
        </method>\
        <method name=\"setValue\">\
            <arg name=\"key\" type=\"s\"/>\
            <arg name=\"value\" type=\"i\"/>\
            <arg name=\"ret\" type=\"i\" direction=\"out\"/>\
        </method>\
        <method name=\"getValue\">\
            <arg name=\"key\" type=\"s\"/>\
            <arg name=\"ret\" type=\"i\" direction=\"out\"/>\
        </method>\
        <signal name=\"signal\">\
            <arg name=\"data\" type=\"u\"/>\
        </signal>\
		  </interface>\n"
		"</node>\n";
	reply = dbus_message_new_method_return(request);
	if (!dbus_message_append_args(reply, DBUS_TYPE_STRING,
			&introspection_data,
			DBUS_TYPE_INVALID))
    {
        fprintf(stderr, "append args error");
        return;
    }
	if (!dbus_connection_send(connection, reply, NULL))
	{
        fprintf(stderr, "Out Of Memory!\n");
        return;
	}
	dbus_message_unref(reply);
}

int gvalue = 0;

// result , module, level  
static void reply_method_get_value(DBusConnection *conn, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter reply_arg;
    DBusMessageIter msg_arg;
    dbus_uint32_t serial = 0;
    int ret = 1;
    char *key = NULL;
    int ret_int = 0;
    int *value = NULL;
 
    //创建返回消息reply
    reply = dbus_message_new_method_return(msg);
    if (!reply) {
        printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
        return;
    }
 
    dbus_message_iter_init_append(reply, &reply_arg);

        /* read param form msg */
    if (!dbus_message_iter_init(msg, &msg_arg)) {
        printf("%s:%d, Message has NO Argument\n", __func__, __LINE__);
        goto _err;
    }
 
    do
    {
        int arg_type = dbus_message_iter_get_arg_type(&msg_arg);
        if (DBUS_TYPE_STRING == arg_type) {
            dbus_message_iter_get_basic(&msg_arg, &key);
            printf("%s:%d, Get Method Argument STRING=  [%s]\n", __func__, __LINE__, key);
            ret = 0;
            value = &gvalue;
        } else {
            ret = 1;
            printf("%s:%d, Argument Type ERROR\n", __func__, __LINE__);
            break;
        } 
    } while (dbus_message_iter_next(&msg_arg));
    
    printf("ret %d\r\n", ret);
    if (ret) {
        goto _err;  
    }

    if (!dbus_message_iter_append_basic(&reply_arg,
                    DBUS_TYPE_INT32, value))
    {
        goto _err; 
    }

    //发送返回消息
    if (!dbus_connection_send(conn, reply, &serial)) {
        printf("%s:%d, Out of Memory\n", __func__, __LINE__);
        goto _err;
    }
 
    dbus_connection_flush(conn);
_err:
    dbus_message_unref(reply);
}

static void reply_method_set_value(DBusConnection *conn, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter reply_arg;
    DBusMessageIter msg_arg;
    dbus_uint32_t serial = 0; 
    int ret = 0;
    char *key = NULL;
    int value = NULL;

    int ret_int = 0;

 
    //创建返回消息reply
    reply = dbus_message_new_method_return(msg);
    if (!reply) {
        printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
        return;
    }
 
    dbus_message_iter_init_append(reply, &reply_arg);

    /* read param form msg */
    if (!dbus_message_iter_init(msg, &msg_arg)) {
        printf("%s:%d, Message has NO Argument\n", __func__, __LINE__);
        goto _err;
    }
 
    do
    {
        int arg_type = dbus_message_iter_get_arg_type(&msg_arg);
        if (DBUS_TYPE_STRING == arg_type) {
            dbus_message_iter_get_basic(&msg_arg, &key);
            printf("%s:%d, Get Method Argument STRING= key [%s]\n", __func__, __LINE__, key);
            
        } else if (DBUS_TYPE_INT32 == arg_type) {
            dbus_message_iter_get_basic(&msg_arg, &value);
            printf("%s:%d, Get Method Argument STRING= value [%d]\n", __func__, __LINE__, value);
            gvalue = value;
            break;
        }
    } while (dbus_message_iter_next(&msg_arg));
    
    if (!dbus_message_iter_append_basic(&reply_arg,
                    DBUS_TYPE_INT32, &ret))
    {
        goto _err;
    }

    printf("ret %d\r\n", ret);
    //发送返回消息
    if (!dbus_connection_send(conn, reply, &serial)) {
        printf("%s:%d, Out of Memory\n", __func__, __LINE__);
        goto _err;
    }
 
    dbus_connection_flush(conn);
_err:
    dbus_message_unref(reply);
}

static void reply_method_call_add(DBusConnection *conn, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter reply_arg;
    DBusMessageIter msg_arg;
    dbus_uint32_t serial = 0; 
    int ret;
    char *ret_str;
    int ret_int = 0;
 
    //创建返回消息reply
    reply = dbus_message_new_method_return(msg);
    if (!reply) {
        printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
        return;
    }
 
    dbus_message_iter_init_append(reply, &reply_arg);
    //此处可以不返回，这样只返回一个参数
    // if (!dbus_message_iter_append_basic(&reply_arg, DBUS_TYPE_STRING, &reply_str)) {
    //     printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
    //     goto _err;
    // }
    
     
    // dbus_message_iter_append_basic(&reply_arg, DBUS_TYPE_STRING, &reply_str);
        /* read param form msg */
    if (!dbus_message_iter_init(msg, &msg_arg)) {
        printf("%s:%d, Message has NO Argument\n", __func__, __LINE__);
        goto _err;
    }
 
    do
    {
        int ret = dbus_message_iter_get_arg_type(&msg_arg);
        if (DBUS_TYPE_STRING == ret) {
            dbus_message_iter_get_basic(&msg_arg, &ret_str);
            printf("%s:%d, Get Method Argument STRING=[%s]\n", __func__, __LINE__, ret_str);
 
            // if (!dbus_message_iter_append_basic(&reply_arg,
            //         DBUS_TYPE_STRING, &ret_str)) {
            //     printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
            //     goto _err;
            // }
        } else if (DBUS_TYPE_INT32 == ret) {
            int value;
            dbus_message_iter_get_basic(&msg_arg, &value);
            printf("%s:%d, Get Method Argument INT32=[%d]\n", __func__, __LINE__, value);
 
            ret_int += value;
            // if (!dbus_message_iter_append_basic(&reply_arg,
            //         DBUS_TYPE_INT32, &ret_int)) {
            //     printf("%s:%d, Out of Memory!\n", __func__, __LINE__);
            //     goto _err;
            // }
        } else {
            printf("%s:%d, Argument Type ERROR\n", __func__, __LINE__);
        }
 
    } while (dbus_message_iter_next(&msg_arg));
    
    if (!dbus_message_iter_append_basic(&reply_arg,
                    DBUS_TYPE_INT32, &ret_int))
                    {

                    }

    //发送返回消息
    if (!dbus_connection_send(conn, reply, &serial)) {
        printf("%s:%d, Out of Memory\n", __func__, __LINE__);
        goto _err;
    }
 
    dbus_connection_flush(conn);
_err:
    dbus_message_unref(reply);
}
 
static void respond_to_Signal(DBusConnection *connection, DBusMessage *msg)
{
    DBusMessageIter args;
    if (!dbus_message_iter_init(msg, &args))
    {
        cout<<"dbus_message_iter_init error, msg has no arguments!"<<endl;
    }
    else if (DBUS_TYPE_UINT32 == dbus_message_iter_get_arg_type(&args))
    {
        dbus_uint32_t ivalue = 0;
        dbus_message_iter_get_basic(&args, &ivalue);
        cout<<"Got signal with value "<<ivalue<<endl;
    }
    else {
        cout<<"unhandle type"<<endl;
    }
}
  
 
static DBusHandlerResult method_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	const char *interface_name = dbus_message_get_interface(message);
	const char *member_name = dbus_message_get_member(message);
	cout<<interface_name <<" "<< member_name<<endl;
	if (0 == strcmp("org.freedesktop.DBus.Introspectable", interface_name) &&
			0 == strcmp("Introspect", member_name))
	{
		respond_to_introspect(connection, message);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
    else if (0 == strcmp(INTERFACE_NAME, interface_name) && 0 == strcmp("Add", member_name))
	{
		reply_method_call_add(connection, message);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
    else if (0 == strcmp(INTERFACE_NAME, interface_name) && 0 == strcmp("setValue", member_name)) {

        reply_method_set_value(connection, message);
		return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (0 == strcmp(INTERFACE_NAME, interface_name) && 0 == strcmp("getValue", member_name)) {
        reply_method_get_value(connection, message);
		return DBUS_HANDLER_RESULT_HANDLED;
    }
	else if (0 == strcmp(INTERFACE_NAME, interface_name) && 0 == strcmp(SIGNAL_NAME, member_name))
	{
        cout<<"signal is called"<<endl;
        respond_to_Signal(connection, message);
        return DBUS_HANDLER_RESULT_HANDLED;
	}
	else
	{
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
}
 
void* wait_signal(void *)
{
    DBusError err;
    DBusMessage *msg;
    DBusMessageIter args;
 
    dbus_error_init(&err);
    DBusConnection *conn;
    if (RES_FAILED == my_dbus_initialization(SERVER_BUS_NAME, &conn))
    {
        exit(1);
    }
    
 
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err))
    {
        cout<<"dbus_bus_add_match err "<<err.message<<endl;
        return NULL;// RES_FAILED;
    }
    cout<<"server start"<<endl;
 
    DBusObjectPathVTable vtable;
    vtable.message_function = method_message;
    vtable.unregister_function = NULL;
    dbus_connection_try_register_object_path(conn,
		    	OBJECTPATH,
                &vtable,
                NULL,
                &err);


    char match_str[64];
    memset(match_str, 0, sizeof(match_str));
    sprintf (match_str, "type='signal'");
    dbus_bus_add_match(conn, match_str,&err);                       // add rule to match
  
 
    while(1)
    {
        if (!dbus_connection_read_write_dispatch(conn, 0))
        {
            fprintf(stderr, "Noe connected now.\n");
            break;
        }
        usleep(1);
    }
}
 
int main()
{
    pthread_t id;
    int ret;

    setenv("DBUS_SESSION_BUS_ADDRESS", "unix:abstract=/dbus-session-address", true);

    // wait_signal(NULL);
    ret = pthread_create(&id, NULL, wait_signal, NULL);
    if (ret != 0)
    {
        cout<<"create pthread error"<<endl;
    }
    pthread_join(id, NULL);
 
    return 0;
}


/*

export DBUS_SESSION_BUS_ADDRESS=unix:abstract=/dbus-session-address
g++ server.cpp -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/include/dbus-1.0/ -ldbus-1 -lpthread -o server

dbus-daemon --session --print-address --fork --print-pid --address=unix:abstract=/dbus-session-address

*/