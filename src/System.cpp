#include <System.hpp>
#include <chrono>

System::System() {
    // bus = new Bus();
    // bus->cia1->setCpu(this);
    // bus->cia2->setCpu(this);
    // bus->vic->setCpu(this);

    bus = new Bus();
    cpu = new CPU(bus);
    cia1 = new CIA1(bus);
    cia2 = new CIA2(bus);
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

    cpu->setCycleCallback([this]() {
        cia1->tick();
        cia2->tick();
        vic->tick();
        // sid->tick();
    });
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
static auto lastTime = std::chrono::high_resolution_clock::now();
static std::chrono::duration<double> accumulatedTime(0);
static int tickCount = 0;
const std::chrono::duration<double> timeThreshold(0.5); // 0.5 seconds

void System::step() {
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = now - lastTime;
    lastTime = now;

    accumulatedTime += delta;
    tickCount++;

    if (accumulatedTime >= timeThreshold) {
        double averageClockSpeed = tickCount / accumulatedTime.count();
        std::cout << "Average Clock speed: " << std::fixed << averageClockSpeed << " Hz" << std::endl;
        accumulatedTime = std::chrono::duration<double>::zero();
        tickCount = 0;
    }

    cpu->executeOnce();
}

