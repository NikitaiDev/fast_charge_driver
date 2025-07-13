#!/bin/bash
set -e

DEVICE="/dev/fast_charge"

echo "[TEST] Проверка доступности $DEVICE"
if [ ! -e $DEVICE ]; then
    echo "[FAIL] Устройство $DEVICE не найдено"
    exit 1
fi

echo "[TEST] Запуск зарядки через write"
echo -n "S" > $DEVICE

read -r charging current voltage error < <(dd if=$DEVICE bs=16 count=1 2>/dev/null | od -An -t u4)
echo "[DEBUG] charging=$charging current=${current}mA voltage=${voltage}mV error=$error"

if [[ "$charging" -ne 1 || "$current" -eq 0 || "$voltage" -eq 0 ]]; then
    echo "[FAIL] Зарядка не запустилась"
    exit 1
fi

echo "[OK] Зарядка запущена"

echo "[TEST] Остановка зарядки через write"
echo -n "P" > $DEVICE

read -r charging current voltage error < <(dd if=$DEVICE bs=16 count=1 2>/dev/null | od -An -t u4)
echo "[DEBUG] charging=$charging current=${current}mA voltage=${voltage}mV error=$error"

if [[ "$charging" -ne 0 || "$current" -ne 0 || "$voltage" -ne 0 ]]; then
    echo "[FAIL] Зарядка не остановилась"
    exit 1
fi

echo "[OK] Зарядка остановлена"
echo "[SUCCESS] Тест пройден"

