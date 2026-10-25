#include <net/protocols/socket.h>
#include <net/protocols/netd.h>
#include <stdio.h>
#include <string.h>

int sync_time_from_ntp(uint32_t *unix_seconds)
{
    int sock = SockOpen(SOCK_KIND_UDP);

    if (sock < 0)
        return sock;
    
    Err bind_result = SockBind(sock, 0);
    if (bind_result != ZUZU_OK) 
    {
        SockClose(sock);
        return bind_result;
    }

    Ipv4Addr ip = ((uint32_t)129) |
        ((uint32_t)6 << 8) |
        ((uint32_t)15 << 16) |
        ((uint32_t)28 << 24);

    uint8_t packet[48];
    memset(packet, 0, sizeof(packet));
    packet[0] = 0b00011011;

    Err send_result = SockSendTo(
        sock,
        ip,
        123,
        packet,
        sizeof(packet)
    );

    if (send_result != ZUZU_OK)
    {
        SockClose(sock);
        return send_result;
    }

    uint8_t response[48];
    uint32_t response_ip;
    uint16_t response_port;

    int received = SockRecvFrom(
        sock,
        response,
        sizeof(response),
        &response_ip,
        &response_port,
        TIMEOUT_INFINITE
    );

    if (received < 48)
    {
        SockClose(sock);

        if (received >= 0)
            return -1;

        return received;
    }

    if (response_ip != ip || response_port != 123)
    {
        SockClose(sock);
        return -1;
    }

    uint32_t ntp_seconds =((uint32_t)response[43]) |
        ((uint32_t)response[42] << 8) |
        ((uint32_t)response[41] << 16) |
        ((uint32_t)response[40] << 24);

    *unix_seconds = ntp_seconds - 2208988800U;

    uint32_t seconds_in_day = *unix_seconds % 86400U;

    uint32_t hours = seconds_in_day / 3600U;
    uint32_t minutes = (seconds_in_day % 3600U) / 60U;
    uint32_t seconds = seconds_in_day % 60U;

    printf("%02u:%02u:%02u\n", hours, minutes, seconds);
    SockClose(sock);
    return 0;
}

int save_time_to_disk(uint32_t *unix_seconds);
int load_time_from_disk(uint32_t *unix_seconds);

int main(void)
{
    uint32_t unix_seconds;
    return sync_time_from_ntp(&unix_seconds);
}