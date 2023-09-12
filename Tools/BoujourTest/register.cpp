//
// Created by aojoie on 9/11/2023.
//

#include <Core/MDNSService.h>
#include <iostream>

using namespace std;


void WaitInput() {
    char a;
    cin >> a;
}

int main()
{

    static uint16_t kServicePort = 4242;

    AN::MDNSService service;

    service.SetRegisterErrorCallback(
            [](const AN::MDNSError &error, void *userInfo)
            {
                cout << "MDNS Error " << error.message << endl;
            });
    service.Register("ANPhoneTool", "_anphonetool._tcp", kServicePort);


    WaitInput();

    service.UnRegister();

    return 0;
}