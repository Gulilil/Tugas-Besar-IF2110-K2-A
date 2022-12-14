// =========================== IMPLEMENTASI ADT UNTUK STRING ===========================

#include <stdio.h>
#include <stdlib.h>
#include "../boolean.h"

#ifndef STRING_H
#define STRING_H

// KONSTRUKTOR
void createStringEmpty(char* str);
// Membuat string (kembali) menjadi string kosong

// Fungsi dan Prosedur lain
void upper(char* str);
// Mengembalikan string dengan huruf kapital


boolean isBlank (char c);
// Memeriksa apakah karakter C adalah blank

int MakeCharToInt(char c);
// Konversi tipe Char ke Integer
// Diasumsikan input selalu benar (merupakan suatu angka)

int stringToInt (char* str);
// Konversi tipe string menjadi integer
// Diasumsikan input selalu benar (merupakan suatu angka)

int lengthString(char *str);
// Mengembalikan panjang sebuah string

boolean isStringEqual (char *str1, char *str2);
// Memeriksa apakah kedua buah string adalah string yang sama

void appendChar(char* str, char c);
// Menggabungkan char dibagian belakang string str
// Kondisi string di declare dengan str[], bukan str*

void appendString(char* str1, char* str2);
// Menggabungkan memasukkan char ke dalam suatu string 
// Diasumsikan c1 tidak kosong

void copyString(char* str1, char* str2);
// Membuat str2 menjadi string yang sama dengan str1

#endif