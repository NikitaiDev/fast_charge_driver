/* TEST_IOCTL_H */
#ifndef TEST_IOCTL_H
#define TEST_IOCTL_H

/* IOCTL commands */
#define FAST_CHARGE_MAGIC 'F'
#define FAST_CHARGE_START   _IO(FAST_CHARGE_MAGIC, 0)
#define FAST_CHARGE_STOP    _IO(FAST_CHARGE_MAGIC, 1)
#define FAST_CHARGE_STATUS  _IOR(FAST_CHARGE_MAGIC, 2, int)

/* Stuct status */
struct fast_charge_status {
    int charging;        /* 0=stop, 1=run */
    int current_mA;
    int voltage_mV;
    int error_code;
};

#endif /* TEST_IOCTL_H */
