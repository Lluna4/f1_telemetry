// CMakeProject2.cpp: define el punto de entrada de la aplicación.
//

#include <print>
#include <frozen/map.h>
#include <frozen/string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <concepts>
#include <cstring>
#include <type_traits>
#include <any>

#pragma comment(lib, "Ws2_32.lib")
#define read_comp_pkt(size, ptr, t) const_for<size>([&](auto i){std::get<i.value>(t) = read_var<std::tuple_element_t<i.value, decltype(t)>>::call(&ptr);});

template<int size, typename T>
struct array_with_size
{
	T array;
	int s;

	static constexpr int getsize()
	{
		return size;
	}
};

template <typename T>
T read_type(char *v)
{
    T a;

    std::memcpy(&a, v, sizeof(T));

    return a;
}

template <typename T>
T read_tyre(char *v)
{
	T tyres = (T)calloc(4, sizeof(std::remove_pointer<T>::type));

	for (int i = 0; i < 4; i++)
	{
		std::memcpy(&tyres[i], v, sizeof(std::remove_pointer<T>::type));
		v += sizeof(std::remove_pointer<T>::type);
	}

	return tyres;
}

template <int size, typename T>
array_with_size<size, T> read_array(char *v)
{
	array_with_size<size, T> a = {0};
	a.array = (T)calloc(size, sizeof(std::remove_pointer<T>::type));

	for (int i = 0; i < size; i++)
	{
		std::memcpy(&a.array[i], v, sizeof(std::remove_pointer<T>::type));
		v += sizeof(std::remove_pointer<T>::type);
	}

	return a;
}

// Template struct for reading variables
template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

template<typename T>
concept IsPointer = std::is_pointer<T>::value;

template<typename T>
struct read_var
{
	static T call(char** v) requires (arithmetic<T>)
    {
        T ret = read_type<T>(*v);
        *v += sizeof(T);
        return ret;
    }
	static T call(char** v) requires (IsPointer<decltype(T::array)>)
    {
        array_with_size<T::getsize(), decltype(T::array)> arr = read_array<T::getsize(), decltype(T::array)>(*v);
        *v += (sizeof(std::remove_pointer<decltype(T::array)>::type) * T::getsize());
        return arr;
    }
};


template <typename Integer, Integer ...I, typename F> constexpr void const_for_each(std::integer_sequence<Integer, I...>, F&& func)
{
    (func(std::integral_constant<Integer, I>{}), ...);
}

template <auto N, typename F> constexpr void const_for(F&& func)
{
    if constexpr (N > 0)
        const_for_each(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
}


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

	while(true)
	{
		size = recvfrom(sock, buffer, 4096, 0, nullptr, nullptr);
		char *first_ptr = buffer;
		constexpr frozen::map<frozen::string, int, 12> header = {
			{"Format", 0},
			{"Year", 1},
			{"MajorVer", 2},
			{"MinorVer", 3},
			{"Version", 4},
			{"Id", 5},
			{"UID", 6},
			{"Time", 7},
			{"Frame id", 8},
			{"Overall frame id", 9},
			{"Car index", 10},
			{"Second car index", 11}
    	};
		std::tuple<
		uint16_t, // m_packetFormat
		uint8_t,  // m_gameYear
		uint8_t,  // m_gameMajorVersion
		uint8_t,  // m_gameMinorVersion
		uint8_t,  // m_packetVersion
		uint8_t,  // m_packetId
		uint64_t, // m_sessionUID
		float,    // m_sessionTime
		uint32_t, // m_frameIdentifier
		uint32_t, // m_overallFrameIdentifier
		uint8_t,  // m_playerCarIndex
		uint8_t   // m_secondaryPlayerCarIndex
		> t;
		constexpr std::size_t size2 = std::tuple_size_v<decltype(t)>;
		read_comp_pkt(size2, buffer, t);

		if (std::get<header.find("Id")->second>(t) == 0)
		{
			std::tuple<float, float, float, float, float, float, int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, float, float, float, float, float, float> pkt_1;
			std::vector<std::tuple<float, float, float, float, float, float, int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, float, float, float, float, float, float>> pkts_1;
			constexpr std::size_t size_motion = std::tuple_size_v<decltype(pkt_1)>;
			constexpr frozen::map<int, frozen::string, size_motion> motion = 
			{
				{0, "m_worldPositionX"},
				{1, "m_worldPositionY"},
				{2, "m_worldPositionZ"},
				{3, "m_worldVelocityX"},
				{4, "m_worldVelocityY"},
				{5, "m_worldVelocityZ"},
				{6, "m_worldForwardDirX"},
				{7, "m_worldForwardDirY"},
				{8, "m_worldForwardDirZ"},
				{9, "m_worldRightDirX"},
				{10, "m_worldRightDirY"},
				{11, "m_worldRightDirZ"},
				{12, "m_gForceLateral"},
				{13, "m_gForceLongitudinal"},
				{14, "m_gForceVertical"},
				{15, "m_yaw"},
				{16, "m_pitch"},
				{17, "m_roll"}
			};
			for (int i = 0; i < 22;i++)
			{
				read_comp_pkt(size_motion, buffer, pkt_1);
				pkts_1.push_back(pkt_1);
			}
		}
		if (std::get<header.find("Id")->second>(t) == 10)
		{
			std::tuple<array_with_size<4, float *>, array_with_size<4, uint8_t *>, array_with_size<4, uint8_t *>, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t> damage_pkt;
			std::vector<std::tuple<array_with_size<4, float *>, array_with_size<4, uint8_t *>, array_with_size<4, uint8_t *>, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>> damage_list;
			constexpr std::size_t size_damage = std::tuple_size_v<decltype(damage_pkt)>;
			constexpr frozen::map<int, frozen::string, size_damage> damage_names = 
			{
				{0, "m_tyresWear"},
				{1, "m_tyresDamage"},
				{2, "m_brakesDamage"},
				{3, "m_frontLeftWingDamage"},
				{4, "m_frontRightWingDamage"},
				{5, "m_rearWingDamage"},
				{6, "m_floorDamage"},
				{7, "m_diffuserDamage"},
				{8, "m_sidepodDamage"},
				{9, "m_drsFault"},
				{10, "m_ersFault"},
				{11, "m_gearBoxDamage"},
				{12, "m_engineDamage"},
				{13, "m_engineMGUHWear"},
				{14, "m_engineESWear"},
				{15, "m_engineCEWear"},
				{16, "m_engineICEWear"},
				{17, "m_engineMGUKWear"},
				{18, "m_engineTCWear"},
				{19, "m_engineBlown"},
				{20, "m_engineSeized"}
			};
			for (int i = 0; i < 22;i++)
			{
				read_comp_pkt(size_damage, buffer, damage_pkt);
				damage_list.push_back(damage_pkt);
			}
			std::tuple<array_with_size<4, float *>, array_with_size<4, uint8_t *>, array_with_size<4, uint8_t *>, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t> pkt_3 = damage_list[std::get<header.find("Car index")->second>(t)];
			std::println("Packet overview:");
			float * tyre_array = std::get<0>(pkt_3).array;
			std::println("Wear RL {}", tyre_array[0]);
			std::println("Wear RR {}", tyre_array[1]);
			std::println("Wear FL {}", tyre_array[2]);
			std::println("Wear FR {}", tyre_array[3]);
		}
		buffer = first_ptr;
	}
}