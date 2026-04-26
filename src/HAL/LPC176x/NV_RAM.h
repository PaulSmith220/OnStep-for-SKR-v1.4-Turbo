// Temporary RAM-backed NVS for LPC176x bring-up.
// This lets the port compile and run, but settings are lost on reset.

#pragma once

#ifndef NV_ENDURANCE
  #define NV_ENDURANCE LOW
#endif

#ifndef E2END
  #define E2END 4095
#endif

class nvs {
  public:
    bool init() { return true; }
    void poll() {}
    bool committed() { return true; }

    byte read(int i) {
      if (i < 0 || i > E2END) return 0;
      return data[i];
    }

    void update(int i, byte j) { write(i, j); }

    void write(int i, byte j) {
      if (i < 0 || i > E2END) return;
      data[i] = j;
    }

    void writeInt(int i, int j) {
      uint8_t *k = (uint8_t*)&j;
      write(i + 0, *k); k++;
      write(i + 1, *k);
    }

    int readInt(int i) {
      uint16_t j;
      uint8_t *k = (uint8_t*)&j;
      *k = read(i + 0); k++;
      *k = read(i + 1);
      return j;
    }

    void writeQuad(int i, byte *v) {
      write(i + 0, *v); v++;
      write(i + 1, *v); v++;
      write(i + 2, *v); v++;
      write(i + 3, *v);
    }

    void readQuad(int i, byte *v) {
      *v = read(i + 0); v++;
      *v = read(i + 1); v++;
      *v = read(i + 2); v++;
      *v = read(i + 3);
    }

    void writeString(int i, char l[]) {
      for (int l1 = 0; l1 < 16; l1++) {
        write(i + l1, *l); l++;
      }
    }

    void readString(int i, char l[]) {
      for (int l1 = 0; l1 < 16; l1++) {
        *l = read(i + l1); l++;
      }
    }

    void writeFloat(int i, float f) { writeQuad(i, (byte*)&f); }

    float readFloat(int i) {
      float f;
      readQuad(i, (byte*)&f);
      return f;
    }

    void writeLong(int i, long l) { writeQuad(i, (byte*)&l); }

    long readLong(int i) {
      long l;
      readQuad(i, (byte*)&l);
      return l;
    }

    void readBytes(uint16_t i, byte *v, uint8_t count) {
      for (int j = 0; j < count; j++) { *v = read(i + j); v++; }
    }

    void writeBytes(uint16_t i, byte *v, uint8_t count) {
      for (int j = 0; j < count; j++) { write(i + j, *v); v++; }
    }

  private:
    byte data[E2END + 1] = {0};
};

nvs nv;
