#include <chrono>
#include <floppy.hpp>
#include <iostream>
#include <system.hpp>

System::System() {
    bus = new Bus();
    serialBus = new SerialBus();
    cpu = new CPU(bus);
    cia1 = new CIA1(bus);
    cia2 = new CIA2(bus, serialBus);
    vic = new VIC(bus);
    sid = new SID();
    input = new Input();
    bus->cia1 = cia1;
    bus->cia2 = cia2;
    bus->vic = vic;
    bus->sid = sid;
    bus->input = input;

    cia1->setCpu(cpu);
    cia2->setCpu(cpu);
    vic->setCpu(cpu);

    Floppy* floppy = new Floppy(serialBus);
    serialBus->devices.push_back(floppy);

    // Initialize clock speed tracking
    lastTime = std::chrono::high_resolution_clock::now();
    accumulatedTime = std::chrono::duration<double>(0);
    cycles = 0;
    timeThreshold = std::chrono::duration<double>(1);
    clockSpeed = 0;
}

System::~System() {
    delete bus;
    delete cpu;
    delete cia1;
    delete cia2;
    delete vic;
    delete sid;
    delete input;
}

void System::loadRoms(const std::string& kernalAndBasicRom, const std::string& characterRom) {
    bus->loadC64rom(kernalAndBasicRom.c_str());
    bus->loadCharacterRom(characterRom.c_str());
}

void System::powerOn() {
    cpu->powerOn();
}

void System::reset() {
    cpu->reset();
}

void System::step() {
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = now - lastTime;
    lastTime = now;

    accumulatedTime += delta;

    if(accumulatedTime >= timeThreshold) {
        double averageClockSpeed = (cpu->cycles - cycles) / accumulatedTime.count();
        clockSpeed = static_cast<int>(averageClockSpeed);
        accumulatedTime = std::chrono::duration<double>::zero();
        cycles = cpu->cycles;
        std::cout << "Clock speed: " << clockSpeed << " Hz" << std::endl;
    }

    cpu->executeOnce();
}
