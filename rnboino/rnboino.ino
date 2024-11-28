#include <Arduino_AdvancedAnalog.h>
#include "SDRAM.h"
#include "tlsf.h"

#define RNBO_USE_FLOAT32
#define RNBO_NOTHROW
#define RNBO_FIXEDLISTSIZE 64
#define RNBO_USECUSTOMALLOCATOR
// RNBO_LIB_PREFIX is very specific to Arduino to have an include path for the RNBO lib
#define RNBO_LIB_PREFIX common

#include "export/rnbo_source.h"

tlsf_t myPool;

#define POOLSIZE (7 * 1024 * 1024)
void* memoryArray = nullptr;

namespace RNBO
{
	namespace Platform
	{

		void *malloc(size_t size)
		{
			return tlsf_malloc(myPool, size);
		}

		void free(void *ptr)
		{
			tlsf_free(myPool, ptr);
		}

		void *realloc(void *ptr, size_t size)
		{
			return tlsf_realloc(myPool, ptr, size);
		}

		void *calloc(size_t count, size_t size)
		{
			auto mem = malloc(count * size);
			memset(mem, 0, count * size);
			return mem;
		}
	}
}

#define VECTORSIZE   (64)
#define SAMPLERATE   (48000)

AdvancedDAC dac0(A13);
AdvancedDAC dac1(A12);

const RNBO::SampleValue convertMul = static_cast<RNBO::SampleValue>(0xFFF);
RNBO::SampleValue outf0[VECTORSIZE] = {};
RNBO::SampleValue outf1[VECTORSIZE] = {};
RNBO::SampleValue* outsf[2];

std::array<uint16_t, VECTORSIZE> outi0;
std::array<uint16_t, VECTORSIZE> outi1;

RNBO::rnbomatic<> rnbo; 

void print_menu() {
  if (!Serial) return;
  Serial.println();
  Serial.println("Parameters:");
  RNBO::ParameterIndex numParams = rnbo.getNumParameters();
  for (RNBO::ParameterIndex i = 0; i < numParams; i++) {
    Serial.print(i);
    Serial.print(" ");
    Serial.print(rnbo.getParameterName(i));

    RNBO::ParameterInfo paramInfo;
    rnbo.getParameterInfo(i, &paramInfo);
    Serial.print(" (");
    Serial.print(paramInfo.min);
    Serial.print(" .. ");
    Serial.print(paramInfo.max);
    Serial.print("): ");
    Serial.print(rnbo.getParameterValue(i));
    Serial.println();
  }
  Serial.println("Eneter Parameter Index and Value (e.g.: '1 0.5'):");
}

void setup() {
  SDRAM.begin();
  memoryArray = SDRAM.malloc(POOLSIZE);
  myPool = tlsf_create_with_pool((void *)memoryArray, POOLSIZE);

  Serial.begin(9600);

  int timeout = 1000;     // Will wait for up to ~1 second for Serial to connect.
  while (!Serial && timeout--) {
    delay(1);
  }

  outsf[0] = outf0;
  outsf[1] = outf1;

  // initialize RNBO, here for example audio samples are allocated in the SDRAM (through our allocator)
  rnbo.initialize();

  // if you do not want this to allocate (and a slightly better performance) consider exporting your code
  // with a fixed audio vector size matching the one you are using (vectorsize)
  rnbo.prepareToProcess(SAMPLERATE, VECTORSIZE, true);

  print_menu();

  if (!dac0.begin(AN_RESOLUTION_12, SAMPLERATE, VECTORSIZE, 64)) {
    Serial.println("Failed to start DAC!");
    while (1);
  }
  if (!dac1.begin(AN_RESOLUTION_12, SAMPLERATE, VECTORSIZE, 64)) {
    Serial.println("Failed to start DAC!");
    while (1);
  }
}

void processAudio() {
  rnbo.process(nullptr, 0, outsf, 2, VECTORSIZE);

  for (size_t i = 0; i < VECTORSIZE; i++) {
    outi0[i] = 0xFFF & static_cast<uint16_t>((outf0[i] * 0.5f + 0.5f) * convertMul);
    outi1[i] = 0xFFF & static_cast<uint16_t>((outf1[i] * 0.5f + 0.5f) * convertMul);
  }
}

void sendParameter(String& str) {
  auto idx = str.indexOf(" ");
  if (idx >= 0) {
    auto paramIdx = str.substring(0, idx).toDouble();
    auto value = str.substring(idx).toDouble();
    rnbo.setParameterValue(paramIdx, value, RNBO::RNBOTimeNow);
    Serial.print("param ");
    Serial.print(paramIdx);
    Serial.print(" set to: ");
    Serial.print(rnbo.getParameterValue(paramIdx));
    Serial.println();
  }
}

void loop() {
  if (Serial && Serial.available() > 0) {
    auto cmd = Serial.readString();
    if (cmd != "\n") {
        sendParameter(cmd);
    }
  } 
  
  if (dac0.available() && dac1.available()) {
    SampleBuffer buf0 = dac0.dequeue();
    SampleBuffer buf1 = dac1.dequeue();

    memcpy(buf0.data(), outi0.data(), sizeof(uint16_t) * VECTORSIZE);
    memcpy(buf1.data(), outi1.data(), sizeof(uint16_t) * VECTORSIZE);

    dac0.write(buf0);
    dac1.write(buf1);

    processAudio();
  }
}
