//
// Created by aojoie on 9/16/2023.
//

#include "UserNotificationCenter.h"

#include "wintoastlib.h"


#ifdef _WIN32
#include <objbase.h>
#endif

UserNotificationCenter::UserNotificationCenter(QObject *parent)
    : QObject(parent)
{
    m_Thread = std::thread([this]
                          {
                               CoInitializeEx(0, COINIT_MULTITHREADED);

                               WinToastLib::WinToast *winToast = WinToastLib::WinToast::instance();
                               winToast->setAppName(L"ANPhoneTool");
                               const auto aumi = WinToastLib::WinToast::configureAUMI(L"AN", L"ANPhoneTool");
                               winToast->setAppUserModelId(aumi);
                               WinToastLib::WinToast::instance()->initialize();


//                               m_EventLoop = new QEventLoop();
//                               m_EventLoop->exec();
                               CoUninitialize();
                           });
}

UserNotificationCenter::~UserNotificationCenter()
{
    while (m_EventLoop == nullptr) {}

    m_EventLoop->quit();
    m_Thread.join();
}

void UserNotificationCenter::showToast(_In_ WinToastLib::WinToastTemplate const& toast, _In_ WinToastLib::IWinToastHandler* eventHandler,
                _Out_opt_ WinToastLib::WinToast::WinToastError* error) {
//    while (m_EventLoop == nullptr) {}
//    QMetaObject::invokeMethod(m_EventLoop, [=]
//                              {
//                                  WinToastLib::WinToast::instance()->showToast(toast, eventHandler, error);
//                              });


    using namespace WinToastLib;
    WinToastTemplate templ = WinToastTemplate(WinToastTemplate::Text01);
    templ.setTextField(L"File Download Complete", WinToastTemplate::FirstLine);

    class WinToastHandlerNone : public WinToastLib::IWinToastHandler {
        public:
                WinToastHandlerNone() {}
        // Public interfaces
        void toastActivated() const override {}
        virtual void toastActivated(int actionIndex) const override {}
        void toastDismissed(WinToastDismissalReason state) const override {}
        void toastFailed() const override {}
    } *handler = new WinToastHandlerNone();


    WinToastLib::WinToast::instance()->showToast(templ, handler);
}


UserNotificationCenter &GetUserNotificationCenter()
{
    static UserNotificationCenter userNotificationCenter;
    return userNotificationCenter;
}
