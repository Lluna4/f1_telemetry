
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
#include <chrono>
#include <filesystem>

#pragma comment(lib, "Ws2_32.lib")

std::map<int, std::string> trackid = {
    {0, "Melbourne"},
    {1, "Paul Ricard"},
    {2, "Shanghai"},
    {3, "Sakhir (Bahrain)"},
    {4, "Catalunya"},
    {5, "Monaco"},
    {6, "Montreal"},
    {7, "Silverstone"},
    {8, "Hockenheim"},
    {9, "Hungaroring"},
    {10, "Spa"},
    {11, "Monza"},
    {12, "Singapore"},
    {13, "Suzuka"},
    {14, "Abu Dhabi"},
    {15, "Texas"},
    {16, "Brazil"},
    {17, "Austria"},
    {18, "Sochi"},
    {19, "Mexico"},
    {20, "Baku (Azerbaijan)"},
    {21, "Sakhir Short"},
    {22, "Silverstone Short"},
    {23, "Texas Short"},
    {24, "Suzuka Short"},
    {25, "Hanoi"},
    {26, "Zandvoort"},
    {27, "Imola"},
    {28, "Portimão"},
    {29, "Jeddah"},
    {30, "Miami"},
    {31, "Las Vegas"},
    {32, "Losail"}
};


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
    std::vector<std::uint32_t> last_laptimes(22);
    std::vector<std::ofstream> files(22);
    std::vector<std::ofstream> lapss(22);
    bool session_name = false;
    bool drivers_mapped = false;

    std::map<int, std::string> drivers;
    
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
             {"Second car index", &typeid(unsigned char)}}};
        Packet pkt = pkt_read(&p, a);
        //std::println("Index {}", pkt.get<unsigned char>("Car index"));
        if (pkt.get<unsigned char>("Id") == 1 && session_name == false && drivers_mapped == true)
        {
            unsigned char index = pkt.get<unsigned char>("Car index");
            char* ptr = p.data;
            packet p2 = { 0, size, size, ptr };
            indexed_map mapp = {
                {{"m_weather", &typeid(unsigned char)},
                 {"m_trackTemperature", &typeid(char)},
                 {"m_airTemperature", &typeid(char)},
                 {"m_totalLaps", &typeid(unsigned char)},
                 {"m_trackLength", &typeid(unsigned short)},
                 {"m_sessionType", &typeid(unsigned char)},
                 {"m_trackId", &typeid(char)}}
            };
            Packet pkt3 = pkt_read(&p2, mapp);
            std::filesystem::create_directory("tyre_data");
            std::filesystem::create_directory("laps");
            auto time = std::chrono::system_clock::now();
            for (auto driver : drivers)
            {
                std::ofstream file;
                std::ofstream laps;
                file.open(std::format("tyre_data\\tyre_info_{}_{:%OH_%OM_%OS}.csv", driver.second, time), std::ios::out | std::ios::app);
                file << "WearRF,WearLF,WearRB,WearLB,Average,time\n";
                laps.open(std::format("laps\\laps_{}_{:%OH_%OM_%OS}.csv", driver.second, time), std::ios::out | std::ios::app);
                laps << "LapTime\n";
                files[driver.first] = std::move(file);
                lapss[driver.first] = std::move(laps);
                last_laptimes[driver.first] = 0;
            }

            session_name = true;
        }
        else if (pkt.get<unsigned char>("Id") == 4 && session_name == false && drivers_mapped == false)
        {
            char* ptr = p.data + 1;
            packet p2 = { 0, size, size, ptr };
            indexed_map mapp = {
                {{"m_aiControlled", &typeid(unsigned char)},
                    {"m_driverId", &typeid(unsigned char)},
                    {"m_networkId", &typeid(unsigned char)},
                    {"m_teamId", &typeid(unsigned char)},
                    {"m_myTeam", &typeid(unsigned char)},
                    {"m_raceNumber", &typeid(unsigned char)},
                    {"m_nationality", &typeid(unsigned char)},
                    {"m_name", &typeid(driver_string)},
                    {"m_yourTelemetry", &typeid(unsigned char)},
                    {"m_showOnlineNames", &typeid(unsigned char)},
                    {"m_platform", &typeid(unsigned char)}}
            };
            std::vector<Packet> pkts = pkt_read(&p, mapp, 22);
            for (int i = 0; i < 22; i++)
            {
                drivers.insert({ i, pkts[i].get<driver_string>("m_name").name });
                std::println("Index {}, driver {}", i, pkts[i].get<driver_string>("m_name").name);
            }
           drivers_mapped = true;
        }
        else if (pkt.get<unsigned char>("Id") == 10 && session_name == true && drivers_mapped == true)
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
                }};
            std::vector<Packet> pkt2 = pkt_read(&p2, a2, 22);
            for (auto driver : drivers)
            {
                index = driver.first;
                float avg = (pkt2[index].get<float>("m_tyresWearFR") + pkt2[index].get<float>("m_tyresWearFL") + pkt2[index].get<float>("m_tyresWearRR") + pkt2[index].get<float>("m_tyresWearRL")) / 4;

                files[driver.first] << std::format("{},{},{},{},{},{}\n", pkt2[index].get<float>("m_tyresWearFR"),
                    pkt2[index].get<float>("m_tyresWearFL"),
                    pkt2[index].get<float>("m_tyresWearRR"),
                    pkt2[index].get<float>("m_tyresWearRL"),
                    avg,
                    pkt.get<float>("Time"));
            }

        }
        else if (pkt.get<unsigned char>("Id") == 2 && session_name == true && drivers_mapped == true)
        {
            unsigned char index = pkt.get<unsigned char>("Car index");
            char* ptr = p.data;
            packet p2 = { 0, size, size, ptr };
            indexed_map mapp = { { {"m_lastLapTimeInMS", &typeid(unsigned int)},
                                 {"m_currentLapTimeInMS", &typeid(unsigned int)},
                                 {"m_sector1TimeInMS", &typeid(unsigned short)},
                                 {"m_sector1TimeMinutes", &typeid(unsigned char)},
                                 {"m_sector2TimeInMS", &typeid(unsigned short)},
                                 {"m_sector2TimeMinutes", &typeid(unsigned char)},
                                 {"m_deltaToCarInFrontInMS", &typeid(unsigned short)},
                                 {"m_deltaToRaceLeaderInMS", &typeid(unsigned short)},
                                 {"m_lapDistance", &typeid(float)},
                                 {"m_totalDistance", &typeid(float)},
                                 {"m_safetyCarDelta", &typeid(float)},
                                 {"m_carPosition", &typeid(unsigned char)},
                                 {"m_currentLapNum", &typeid(unsigned char)},
                                 {"m_pitStatus", &typeid(unsigned char)},
                                 {"m_numPitStops", &typeid(unsigned char)},
                                 {"m_sector", &typeid(unsigned char)},
                                 {"m_currentLapInvalid", &typeid(unsigned char)},
                                 {"m_penalties", &typeid(unsigned char)},
                                 {"m_totalWarnings", &typeid(unsigned char)},
                                 {"m_cornerCuttingWarnings", &typeid(unsigned char)},
                                 {"m_numUnservedDriveThroughPens", &typeid(unsigned char)},
                                 {"m_numUnservedStopGoPens", &typeid(unsigned char)},
                                 {"m_gridPosition", &typeid(unsigned char)},
                                 {"m_driverStatus", &typeid(unsigned char)},
                                 {"m_resultStatus", &typeid(unsigned char)},
                                 {"m_pitLaneTimerActive", &typeid(unsigned char)},
                                 {"m_pitLaneTimeInLaneInMS", &typeid(unsigned short)},
                                 {"m_pitStopTimerInMS", &typeid(unsigned short)},
                                 {"m_pitStopShouldServePen", &typeid(unsigned char)} } };
            std::vector<Packet> pkt2 = pkt_read(&p2, mapp, 22);

            for (auto driver : drivers)
            {
                index = driver.first;
                unsigned int lap = pkt2[index].get<unsigned int>("m_lastLapTimeInMS");
                std::uint32_t last_laptime = last_laptimes[driver.first];
                if (lap != last_laptime)
                {
                    last_laptime = lap;
                    auto lap_time = std::chrono::milliseconds(lap);
                    auto min = std::chrono::duration_cast<std::chrono::minutes>(lap_time);
                    lap_time -= min;
                    auto sec = std::chrono::duration_cast<std::chrono::seconds>(lap_time);
                    lap_time -= sec;
                    lapss[driver.first] << std::format("{}:{}.{}", min.count(), sec.count(), lap_time.count()) << std::endl;
                    //std::println("Last lap: {}:{}.{}", min.count(), sec.count(), lap_time.count());
                    last_laptimes[driver.first] = last_laptime;
                }
            }
        }
        memset(buffer, 0, 4096);
    }
    //file.close();
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
