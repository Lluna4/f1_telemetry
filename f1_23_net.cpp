
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "packet_read.hpp"
#include <vector>
#include <any>
#include <map>
#include <print>
#include <fstream>
#include <format>

#pragma comment(lib, "Ws2_32.lib")


int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    char* buffer = (char*)calloc(4096, sizeof(char));
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {
        AF_INET,
        htons((u_short)20777),
        0
    };
    int size = 0;
    int addr_size = sizeof(addr);
    bind(sock, (sockaddr*)&addr, addr_size);
    std::ofstream file;
    file.open("a.csv", std::ios::out | std::ios::app);

    file << "WearRF,WearLF,WearRB,WearLB,time\n";
    while (true)
    {
        size = recvfrom(sock, buffer, 4096, 0, nullptr, nullptr);
       
        if (size == -1)
            std::println("Error {}", WSAGetLastError());
        packet p = { 0, size, size, buffer };
        indexed_map a = {
            {{"Format", &typeid(unsigned short)},
             {"Year", &typeid(unsigned char)},
             {"MajorVer", &typeid(unsigned char)},
             {"MinorVer", &typeid(unsigned char)},
             {"Version", &typeid(unsigned char)},
             {"Id", &typeid(unsigned char)},
             {"UID", &typeid(unsigned long)},
             {"Time", &typeid(float)},
             {"Frame id", &typeid(unsigned int)},
             {"Overall frame id", &typeid(unsigned int)},
             {"Car index", &typeid(unsigned char)},
             {"Second car index", &typeid(unsigned char)}},
             {"Format", "Year", "MajorVer", "MinorVer",
              "Version", "Id", "UID", "Time", "Frame id", "Overall frame id",
              "Car index", "Second car index"} };
        Packet pkt = pkt_read(&p, a);
        //std::println("Index {}", pkt.get<unsigned char>("Car index"));
        if (pkt.get<unsigned char>("Id") == 10)
        {
            unsigned char index = pkt.get<unsigned char>("Car index");
            char *ptr = p.data;
            packet p2 = { 0, size, size, ptr };
            indexed_map a2 = {
                {{"m_tyresWearRL", &typeid(float)},
                 {"m_tyresWearRR", &typeid(float)},
                 {"m_tyresWearFL", &typeid(float)},
                 {"m_tyresWearFR", &typeid(float)},
                 {"m_tyresDamageFL", &typeid(unsigned char)},
                 {"m_tyresDamageFR", &typeid(unsigned char)},
                 {"m_tyresDamageRL", &typeid(unsigned char)},
                 {"m_tyresDamageRR", &typeid(unsigned char)},
                 {"m_brakesDamageFL", &typeid(unsigned char)},
                 {"m_brakesDamageFR", &typeid(unsigned char)},
                 {"m_brakesDamageRL", &typeid(unsigned char)},
                 {"m_brakesDamageRR", &typeid(unsigned char)},
                 {"m_frontLeftWingDamage", &typeid(unsigned char)},
                 {"m_frontRightWingDamage", &typeid(unsigned char)},
                 {"m_rearWingDamage", &typeid(unsigned char)},
                 {"m_floorDamage", &typeid(unsigned char)},
                 {"m_diffuserDamage", &typeid(unsigned char)},
                 {"m_sidepodDamage", &typeid(unsigned char)},
                 {"m_drsFault", &typeid(unsigned char)},
                 {"m_ersFault", &typeid(unsigned char)},
                 {"m_gearBoxDamage", &typeid(unsigned char)},
                 {"m_engineDamage", &typeid(unsigned char)},
                 {"m_engineMGUHWear", &typeid(unsigned char)},
                 {"m_engineESWear", &typeid(unsigned char)},
                 {"m_engineCEWear", &typeid(unsigned char)},
                 {"m_engineICEWear", &typeid(unsigned char)},
                 {"m_engineMGUKWear", &typeid(unsigned char)},
                 {"m_engineTCWear", &typeid(unsigned char)},
                 {"m_engineBlown", &typeid(unsigned char)},
                 {"m_engineSeized", &typeid(unsigned char)}
                },
                {"m_tyresWearRL", "m_tyresWearRR", "m_tyresWearFL", "m_tyresWearFR",
                 "m_tyresDamageRL", "m_tyresDamageRR", "m_tyresDamageFL", "m_tyresDamageFR",
                 "m_brakesDamageRL", "m_brakesDamageRR", "m_brakesDamageFL", "m_brakesDamageFR",
                 "m_frontLeftWingDamage", "m_frontRightWingDamage", "m_rearWingDamage", "m_floorDamage",
                 "m_diffuserDamage", "m_sidepodDamage", "m_drsFault", "m_ersFault", "m_gearBoxDamage",
                 "m_engineDamage", "m_engineMGUHWear", "m_engineESWear", "m_engineCEWear", "m_engineICEWear",
                 "m_engineMGUKWear", "m_engineTCWear", "m_engineBlown", "m_engineSeized"}};
            std::vector<Packet> pkt2 = pkt_read(&p2, a2, 22);
            file << std::format("{},{},{},{},{}\n", pkt2[index].get<float>("m_tyresWearFR"),
                pkt2[index].get<float>("m_tyresWearFL"),
                pkt2[index].get<float>("m_tyresWearRR"),
                pkt2[index].get<float>("m_tyresWearRL"),
                pkt.get<float>("Time"));
        }
        memset(buffer, 0, 4096);
    }
    file.close();
    closesocket(sock);
    return 0;
}

// Ejecutar programa: Ctrl + F5 o menú Depurar > Iniciar sin depurar
// Depurar programa: F5 o menú Depurar > Iniciar depuración

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de código fuente
//   3. Use la ventana de salida para ver la salida de compilación y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de código, o a Proyecto > Agregar elemento existente para agregar archivos de código existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
