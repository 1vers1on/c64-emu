#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cpu.hpp>

int main() {
    CPU cpu;

    cpu.bus->loadC64rom("c64.kernel.bin");
    cpu.bus->loadCharacterRom("c64.chrom.bin");

    cpu.powerOn();

    std::cout << std::hex << static_cast<int>(cpu.bus->readWord(0xFFFC)) << "\n";

    std::cout << "rom loaded and mapped successfully!" << "\n";
    std::ios::sync_with_stdio(false);

    while (true) {
        cpu.executeOnce();
        cpu.bus->cia1->updateTimers(cpu.cycles);
        cpu.bus->vic->tick(cpu.cycles);
        if (cpu.bus->vic->irqPending) {
            cpu.triggerIrq();
            cpu.bus->vic->irqPending = false;
        }
    }

    return 0;
}
