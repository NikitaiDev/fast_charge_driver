#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "test_ioctl.h"

int main(void)
{
    int fd = open("/dev/fast_charge", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("[TEST] START_CHARGE через ioctl\n");
    if (ioctl(fd, FAST_CHARGE_START) < 0) {
        perror("ioctl START");
        close(fd);
        return 1;
    }

    struct fast_charge_status st;
    printf("[TEST] GET_STATUS через ioctl\n");
    if (ioctl(fd, FAST_CHARGE_STATUS, &st) < 0) {
        perror("ioctl STATUS");
        close(fd);
        return 1;
    }

    printf("[DEBUG] charging=%d current=%dmA voltage=%dmV error=%d\n",
           st.charging, st.current_mA, st.voltage_mV, st.error_code);

    if (st.charging != 1 || st.current_mA == 0 || st.voltage_mV == 0) {
        printf("[FAIL] Статус после START неверный\n");
        close(fd);
        return 1;
    }

    printf("[TEST] STOP_CHARGE через ioctl\n");
    if (ioctl(fd, FAST_CHARGE_STOP) < 0) {
        perror("ioctl STOP");
        close(fd);
        return 1;
    }

    if (ioctl(fd, FAST_CHARGE_STATUS, &st) < 0) {
        perror("ioctl STATUS");
        close(fd);
        return 1;
    }

    printf("[DEBUG] charging=%d current=%dmA voltage=%dmV error=%d\n",
           st.charging, st.current_mA, st.voltage_mV, st.error_code);

    if (st.charging != 0 || st.current_mA != 0 || st.voltage_mV != 0) {
        printf("[FAIL] Статус после STOP неверный\n");
        close(fd);
        return 1;
    }

    printf("[SUCCESS] ioctl_test завершён успешно.\n");
    close(fd);
    return 0;
}

