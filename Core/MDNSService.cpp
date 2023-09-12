//
// Created by aojoie on 9/11/2023.
//

#include "MDNSService.h"
#include "Utilities/Log.h"

#include <unordered_map>

namespace AN
{

std::wstring TranslateErrorCodeW(HRESULT hr) noexcept {
    wchar_t *msgBuf = nullptr;
    DWORD msgLen  = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            msgBuf,
            0,
            nullptr);

    if (msgLen == 0) {
        return L"Unidentified error code";
    }

    std::wstring errorString(msgBuf);
    LocalFree(msgBuf);
    return errorString;
}

std::string TranslateErrorCode(HRESULT hr) noexcept {
    std::wstring wText = TranslateErrorCodeW(hr);
    std::string text;
    text.resize(WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr));
    WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, text.data(), (int)text.size(), nullptr, nullptr);
    return text;
}

std::string GetLastErrorString() noexcept {
    return TranslateErrorCode(GetLastError());
}


static std::unordered_map<uint64_t, const char *> s_DNSServiceErrorMap;

static void InitializeDNSServiceErrorMap()
{
    static bool inited = false;
    if (inited) return;
    else
        inited = true;

#define CHECK_ERROR(type, error) do {\
    if (error != kDNSServiceErr_NoError) \
    { \
        Emit##type##Error(error, GetDNSServiceErrorString(error)); \
        return; \
    } } while (0)


#define CHECK_ERROR_SELF(type, error, self) do {\
    if (error != kDNSServiceErr_NoError) \
    { \
        self->Emit##type##Error(error, GetDNSServiceErrorString(error)); \
        return; \
    } } while (0)


#define CHECK_ERROR_FALSE(type, error) do { \
    if (error != kDNSServiceErr_NoError) \
    { \
        Emit##type##Error(error, GetDNSServiceErrorString(error)); \
        return false; \
    } } while (0)

#define MAP(map) s_DNSServiceErrorMap[map] = #map

    MAP(kDNSServiceErr_NoError);
    MAP(kDNSServiceErr_Unknown);
    MAP(kDNSServiceErr_NoSuchName);
    MAP(kDNSServiceErr_NoMemory);
    MAP(kDNSServiceErr_BadParam);
    MAP(kDNSServiceErr_BadReference);
    MAP(kDNSServiceErr_BadState);
    MAP(kDNSServiceErr_BadFlags);
    MAP(kDNSServiceErr_Unsupported);
    MAP(kDNSServiceErr_NotInitialized);
    MAP(kDNSServiceErr_AlreadyRegistered);
    MAP(kDNSServiceErr_NameConflict);
    MAP(kDNSServiceErr_Invalid);
    MAP(kDNSServiceErr_Firewall);
    MAP(kDNSServiceErr_Incompatible);
    MAP(kDNSServiceErr_BadInterfaceIndex);
    MAP(kDNSServiceErr_Refused);
    MAP(kDNSServiceErr_NoSuchRecord);
    MAP(kDNSServiceErr_NoAuth);
    MAP(kDNSServiceErr_NoSuchKey);
    MAP(kDNSServiceErr_NATTraversal);
    MAP(kDNSServiceErr_DoubleNAT);
    MAP(kDNSServiceErr_BadTime);
    MAP(kDNSServiceErr_BadSig);
    MAP(kDNSServiceErr_BadKey);
    MAP(kDNSServiceErr_Transient);
    MAP(kDNSServiceErr_ServiceNotRunning);
    MAP(kDNSServiceErr_NATPortMappingUnsupported);
    MAP(kDNSServiceErr_NATPortMappingDisabled);
    MAP(kDNSServiceErr_NoRouter);
    MAP(kDNSServiceErr_PollingMode);
    MAP(kDNSServiceErr_Timeout);

#undef MAP
}

const char *GetDNSServiceErrorString(uint64_t code)
{
    if (s_DNSServiceErrorMap.count(code) == 0)
    {
        return "kDNSServiceErr_Unknown";
    }
    return s_DNSServiceErrorMap[code];
}

void MDNSService::ServiceThreadLoop()
{
    for (;;)
    {
        if (m_ThreadRunning)
        {
            std::vector<int> handles;
            std::vector<DNSServiceRef> services;

            fd_set set;
            FD_ZERO(&set);

            FD_SET(m_SignalSocket, &set);

            {
                std::lock_guard lock(m_ServiceInfosMutex);
                for (const auto &info : m_ServicesInfos)
                {
                    handles.push_back(info.fd);
                    services.push_back(info.serviceRef);

                    FD_SET(info.fd, &set);
                }
            }


            // Set the timeout (or use NULL for blocking)
            struct timeval timeout;
            timeout.tv_sec = 10;  // Set the timeout to 10 seconds
            timeout.tv_usec = 0;

            int result = select(0, &set, nullptr, nullptr, &timeout);

            if (!m_ThreadRunning)
            {
                break;
            }

            if (result == SOCKET_ERROR) {
                // Handle select error
                int error = WSAGetLastError();
                // Handle the error code
            } else if (result == 0) {
                // Timeout occurred, no sockets have events
            } else {
                for (int i = 0; i < handles.size(); ++i) {
                    if (FD_ISSET(handles[i], &set))
                    {
                        // if service is removed
                        bool removed = true;
                        {
                            std::lock_guard lock(m_ServiceInfosMutex);
                            for (const auto &info : m_ServicesInfos)
                            {
                                if (info.serviceRef == services[i])
                                {
                                    removed = false;
                                }
                            }
                        }

                        if (removed)
                        {
                            continue;
                        }

                        // result by callback ?
                        DNSServiceProcessResult(services[i]);
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
}

MDNSService::MDNSService()
    : m_RegisterErrorCallback(),
      m_DiscoverErrorCallback(),
      m_BrowseCallback(),
      m_UserInfo(),
      m_DnsServiceRef(),
      m_DnsBrowsingRef(),
      m_ThreadRunning()
{
    InitializeDNSServiceErrorMap();
}

MDNSService::~MDNSService()
{
    StopServiceThread();
    UnRegister();
    StopDiscover();
}

void MDNSService::StartServiceThread()
{
    m_SignalSocket = socket(AF_INET, SOCK_STREAM, 0);
    m_ThreadRunning = true;
    m_ServiceThread = std::thread(
            [this]
            {
                ServiceThreadLoop();
            });
}
void MDNSService::StopServiceThread()
{
    if (m_ThreadRunning)
    {
        m_ThreadRunning = false;
        closesocket(m_SignalSocket);
        if (m_ServiceThread.joinable())
        {
            m_ServiceThread.join();
        }
    }
}

void MDNSService::EmitRegisterError(int code, const char *msg)
{
    m_LastError = { code, msg };
    if (m_RegisterErrorCallback)
    {
        m_RegisterErrorCallback(m_LastError, m_UserInfo);
    }
}

void MDNSService::EmitDiscoverError(int code, const char *msg)
{
    m_LastError = { code, msg };
    if (m_DiscoverErrorCallback)
    {
        m_DiscoverErrorCallback(m_LastError, m_UserInfo);
    }
}

void MDNSService::DNSServiceRegisterReplyCallback(DNSServiceRef       sdRef,
                                                  DNSServiceFlags     flags,
                                                  DNSServiceErrorType errorCode,
                                                  const char         *name,
                                                  const char         *regtype,
                                                  const char         *domain,
                                                  void               *context)
{
    if (errorCode != kDNSServiceErr_NoError)
    {
        ((MDNSService *) context)->EmitRegisterError(errorCode, GetDNSServiceErrorString(errorCode));
    }

    AN_LOG(Info, "service register reply %s, %s, %s", name, regtype, domain);
}

bool AN::MDNSService::Register(const char *name, const char *regtype, int port)
{
    if (m_DnsServiceRef)
    {
        UnRegister();
    }

    DNSServiceErrorType error = DNSServiceRegister(
            &m_DnsServiceRef,
            0,
            0,// all interfaces
            name,
            regtype,
            nullptr,// default domin
            nullptr,// default host
            port,
            0,      // txt record length
            nullptr,// txt record
            DNSServiceRegisterReplyCallback,
            this);

    CHECK_ERROR_FALSE(Register, error);

    AddService(m_DnsServiceRef);

    StartServiceThread();

    return true;
}

void MDNSService::UnRegister()
{
    StopServiceThread();
    if (m_DnsServiceRef)
    {
        DNSServiceRefDeallocate(m_DnsServiceRef);
        m_DnsServiceRef = nullptr;
    }
    RemoveService(m_DnsServiceRef);
}

bool MDNSService::StartDiscover(const char *regtype, AN::MDNSDiscoverCallback callback, void *userData)
{
    if (m_DnsBrowsingRef)
    {
        StopDiscover();
    }

    m_BrowseCallback = callback;

    DNSServiceErrorType error;

    error = DNSServiceBrowse(
            &m_DnsBrowsingRef, 0, 0, regtype, nullptr,
            [](DNSServiceRef sdRef,
               DNSServiceFlags flags,
               uint32_t interfaceIndex,
               DNSServiceErrorType errorCode,
               const char *serviceName,
               const char *regtype,
               const char *replyDomain,
               void *context)
            {
                MDNSService *self = (MDNSService *)context;
                DNSServiceErrorType error;

                CHECK_ERROR_SELF(Discover, errorCode, self);

                if (flags & kDNSServiceFlagsAdd)
                {

                    DNSServiceRef resolveServiceRef;
                    ResolveInfo *resolveInfo = new ResolveInfo{};

                    error = DNSServiceResolve(
                            &resolveServiceRef, 0, interfaceIndex, serviceName, regtype, replyDomain,
                            [](DNSServiceRef        sdRef,
                               DNSServiceFlags      flags,
                               uint32_t             interfaceIndex,
                               DNSServiceErrorType  errorCode,
                               const char          *fullname,
                               const char          *hosttarget,
                               uint16_t             port, /* In network byte order */
                               uint16_t             txtLen,
                               const unsigned char *txtRecord,
                               void                *context)
                            {
                                ResolveInfo *resolveInfo = (ResolveInfo *) context;
                                MDNSService *self = resolveInfo->self;

                                CHECK_ERROR_SELF(Discover, errorCode, self);

                                if (self->m_BrowseCallback)
                                {
                                    self->m_BrowseCallback(resolveInfo->serviceName.c_str(),
                                                           kMDNSDiscoverAdd, hosttarget, port, {}, self->m_UserInfo);
                                }

                                self->RemoveResolveInfo(resolveInfo);
                                self->RemoveService(sdRef);

                                DNSServiceRefDeallocate(sdRef);
                            },
                            resolveInfo);

                    if (error != kDNSServiceErr_NoError)
                    {
                        self->EmitDiscoverError(error, GetDNSServiceErrorString(error));
                        delete resolveInfo;
                        return;
                    }

                    resolveInfo->self = self;
                    resolveInfo->serviceName = serviceName;
                    resolveInfo->resolveRef = resolveServiceRef;

                    self->AddResolveInfo(resolveInfo);
                    self->AddService(resolveServiceRef);
                }
                else
                {
                    // A service was removed
                    self->m_BrowseCallback(serviceName, kMDNSDiscoverRemove, nullptr, 0, {}, self->m_UserInfo);
                }
            },
            this);

    CHECK_ERROR_FALSE(Discover, error);

    AddService(m_DnsBrowsingRef);

    StartServiceThread();

    return true;
}

void MDNSService::StopDiscover()
{
    StopServiceThread();
    if (m_DnsBrowsingRef)
    {
        DNSServiceRefDeallocate(m_DnsBrowsingRef);
        m_DnsBrowsingRef = nullptr;
    }
    RemoveService(m_DnsBrowsingRef);
}

void MDNSService::AddResolveInfo(MDNSService::ResolveInfo *info)
{
    std::lock_guard lock(m_ResolveInfosMutex);
    m_ResolveInfos.push_back(info);
}

void MDNSService::RemoveResolveInfo(MDNSService::ResolveInfo *info)
{
    std::lock_guard lock(m_ResolveInfosMutex);
    for (auto it = m_ResolveInfos.begin(); it != m_ResolveInfos.end(); ++it)
    {
        if (*it == info)
        {
            m_ResolveInfos.erase(it);
            break;
        }
    }

    delete info;
}

void MDNSService::AddService(DNSServiceRef serviceRef)
{
    std::lock_guard lock(m_ServiceInfosMutex);
    m_ServicesInfos.push_back({ DNSServiceRefSockFD(serviceRef), serviceRef });
}

void MDNSService::RemoveService(DNSServiceRef serviceRef)
{
    std::lock_guard lock(m_ServiceInfosMutex);

    for (auto it = m_ServicesInfos.begin(); it != m_ServicesInfos.end(); ++it)
    {
        if (it->serviceRef == serviceRef)
        {
            m_ServicesInfos.erase(it);
            break;
        }
    }
}

}// namespace AN