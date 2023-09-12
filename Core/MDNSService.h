//
// Created by aojoie on 9/11/2023.
//

#pragma once

#include <dns_sd.h>

#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <thread>


#ifdef _WIN32
#include <windows.h>
#endif

namespace AN
{

enum MDNSErrorType
{
    kMDNSNoError = 0,
    kMDNSGetAdapterFail,
    kMDNSOpenClientSockFail
};

struct MDNSError
{
    int         code;
    std::string message;
};

enum MDNSDiscoverState
{
    kMDNSDiscoverAdd = 0,
    kMDNSDiscoverRemove
};

typedef void (*MDNSErrorCallback)(const MDNSError &error, void *userInfo);

typedef void (*MDNSDiscoverCallback)(const char *name,
                                     MDNSDiscoverState state,
                                     const char *hosttarget,
                                     uint16_t port,
                                     const MDNSError &error,
                                     void *userInfo);

class MDNSService
{
    MDNSError          m_LastError;

    MDNSErrorCallback    m_RegisterErrorCallback;
    MDNSErrorCallback    m_DiscoverErrorCallback;
    MDNSDiscoverCallback m_BrowseCallback;

    void *m_UserInfo;

    DNSServiceRef    m_DnsServiceRef;
    DNSServiceRef    m_DnsBrowsingRef;

    struct ResolveInfo
    {
        MDNSService  *self;
        DNSServiceRef resolveRef;
        std::string   serviceName;
    };

    std::mutex                 m_ResolveInfosMutex;
    std::vector<ResolveInfo *> m_ResolveInfos;


    struct ServiceInfo {
        int fd;
        DNSServiceRef serviceRef;
    };

    std::mutex               m_ServiceInfosMutex;
    std::vector<ServiceInfo> m_ServicesInfos;

    SOCKET m_SignalSocket;

    std::atomic_bool m_ThreadRunning;
    std::thread m_ServiceThread;

    void ServiceThreadLoop();

    void EmitRegisterError(int code, const char *msg);
    void EmitDiscoverError(int code, const char *msg);

    static void DNSServiceRegisterReplyCallback(DNSServiceRef       sdRef,
                                                DNSServiceFlags     flags,
                                                DNSServiceErrorType errorCode,
                                                const char         *name,
                                                const char         *regtype,
                                                const char         *domain,
                                                void               *context);

    void AddResolveInfo(ResolveInfo *info);
    void RemoveResolveInfo(ResolveInfo *info);

    void AddService(DNSServiceRef serviceRef);
    void RemoveService(DNSServiceRef serviceRef);

    void StartServiceThread();
    void StopServiceThread();

public:

    MDNSService();

    ~MDNSService();

    void SetRegisterErrorCallback(MDNSErrorCallback callback) { m_RegisterErrorCallback = callback; }
    void SetDiscoverErrorCallback(MDNSErrorCallback callback) { m_DiscoverErrorCallback = callback; }

    void SetUserInfo(void *userInfo) { m_UserInfo = userInfo; }

    bool Register(const char *name, const char *regtype, int port);

    void UnRegister();

    bool StartDiscover(const char *regtype, MDNSDiscoverCallback callback, void *userData);

    void StopDiscover();

    MDNSError GetLastError() const { return m_LastError; }
};

}// namespace AN
