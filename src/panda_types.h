#ifndef PANDA_TYPES_H
#define PANDA_TYPES_H

#include <Arduino.h>

// Definito in un header separato (non nel .ino) perché Arduino genera
// automaticamente i prototipi delle funzioni in cima al file .ino,
// PRIMA di qualunque struct definito nel file stesso — causando
// l'errore "DataOra does not name a type" se lo struct resta nel .ino.
struct DataOra {
  uint8_t secondi, minuti, ore, giorno, mese;
  uint16_t anno;
};

#endif
