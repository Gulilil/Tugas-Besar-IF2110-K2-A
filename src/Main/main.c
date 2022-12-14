#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "submain.c"


int main () {
    char command[MAX_COMMAND] = "\0";
    boolean program = true;
    int idx;
    ASCIIArt();
    printf("Input 'START' untuk melakukan program.\n");
    getInput(command);

    // Menggunakan mesin kata untuk membaca
    STARTWORD(command, &idx);
    /* Karena hanya satu kata (hanya menerima 'START'), 
    cukup STARTWORD saja yang dijalankan */

    while (!isWordStringEqual(currentWord, "START")){
        printf("Input selain 'START' tidak diterima.\n");
        getInput(command);
        STARTWORD(command, &idx);
    }

    // ==== Mulai Konfigurasi ====

    printf("====================================================\n");
    printf("==============      KONFIGURASI      ===============\n");
    printf("====================================================\n");

    // Konfigurasi Simulator
    Simulator sim;
    ReadSimulator(&sim);
    printf("-> Konfigurasi Simulator - DONE\n");

    // Konfigurasi Waktu
    TIME currentTime;
    printf("Masukkan waktu awal dalam format (<hari> <jam> <menit>) : ");
    BacaTIME(&currentTime);
    TIME boundariesTime; Day(boundariesTime) = 0; Hour(boundariesTime) = 0; Minute(boundariesTime) = 0;
    printf("-> Konfigurasi Waktu - DONE\n");

    // Konfigurasi Makanan
    ListStatik foodList;
    foodList = konfigMakanan();
    int foodListLength = listLength(foodList);
    printf("-> Konfigurasi Makanan - DONE\n");
    
    // Konfigurasi Resep
    Cookbook bukuResep;
    bukuResep = konfigResep();
    printf("-> Konfigurasi Resep - DONE\n");

    // Konfigurasi Peta
    Matrix map;
    map = konfigMap();
    POINT startLoc = searchCharInMatrix(map, 'S');
    Absis(Lokasi(sim)) = Ordinat(startLoc);
    Ordinat(Lokasi(sim)) = Absis(startLoc);
    printf("-> Konfigurasi Peta - DONE\n");

    // Konfigurasi Stack
    Stack SUndo, SRedo;
    CreateEmpty(&SRedo);
    CreateEmpty(&SUndo);
    printf("-> Konfigurasi Stack - DONE\n");

    //Konfigurasi state
    state currentState, dummyredo;
    initState(&currentState,sim,currentTime);
    printf("-> Konfigurasi State - DONE\n");

    //Konfigurasi Kulkas
    Kulkas k;
    CreateKulkas(&k);
    printf("-> Konfigurasi Kulkas - DONE\n");

    // Dummy (untuk mencegah error)
    fgets(command, MAX_COMMAND, stdin);

    // Konfigurasi variabel global
    int i;
    int count;
    int totalcommand = 0; // untuk bisa berapa kali undo
    int totalundo = 0; // untuk bisa berapa kali redo
    int jumlahredo = 0; //untuk mereset redo
    int idxredo = 0; //untuk mereset redo
    boolean salahinput = true; // untuk tidak jadi push ke undo
    boolean subprogram = false;
    int idxFood;
    Makanan dumpMkn;
    Makanan tempMkn;
    Word currentAct;
    boolean notifUndo = false;
    Word undoAct;
    boolean notifRedo = false;
    Word redoAct;
    int notifCount = 0;
    int deliveredCount= 0 ;
    int processedCount = 0;
    int expiredCount = 0;
    ListStatik notifList;
    CreateListStatik(&notifList);
    POINT fridgePoint;


    // =========== PENJALANAN PROGRAM UTAMA ===========
    while(program){
        printf("====================================================\n");
        printf("==============       MAIN MENU       ===============\n");
        printf("====================================================\n");

        // Display simulator dan map
        DisplaySimulator(currentState.sub1);
        printf("Waktu: ");
        TulisTIME(currentState.sub2); 
        printf("Notifikasi : "); // Ini nanti ditambahin seiring berjalan waktu
        if (notifCount == 0) {
            printf("-\n");
        } else {
            printf("\n");
            int notifNumber = 1;
            while (notifCount > 0) {
                if (notifUndo) {
                    printf("   %d. ", notifNumber);
                    printWord(undoAct);
                    printf(" tidak jadi dilakukan.\n");
                    notifUndo = false;
                } else if (notifRedo) {
                    printf("   %d. ", notifNumber);
                    printWord(redoAct);
                    printf(" kembali dilakukan.\n");
                    notifRedo = false;
                } else {
                    deleteFirst(&notifList, &dumpMkn);
                    printf("   %d. ", notifNumber);
                    printWord(nameMkn(dumpMkn));
                    if (deliveredCount > 0) {
                        printf(" sudah diterima oleh BNMO!\n");
                        deliveredCount--;
                    } else if (processedCount > 0) {
                        printf(" sudah selesai diproses dan dimasukkan ke dalam inventory.\n");
                        processedCount--;
                    } else {
                        printf(" kedaluwarsa.. :(\n");
                        expiredCount--;
                    }
                }
                notifCount--;
                notifNumber++;
            }
        }

        displayMatrix(map);
        
        // validAction digunakan untuk menandakan apakah suatu aksi menghabiskan waktu
        // Beberapa action dianggap tidak menghabiskan waktu
        boolean validAction = true;

        printf("Silahkan masukkan command yang ingin dilakukan.\n");
        printf("Masukkan 'HELP' untuk melihat list command yang dapat digunakan.\n");
        printf("Command: ");
        
        // Meminta input command setelah program dimulai
        getInput(command);
        STARTWORD(command, &idx);

        // WAIT, MOVE, FRIDGE boleh memiliki input lanjutan dibelakangnya
        if (isWordStringEqual(currentWord, "WAIT")){
            printf("====================================================\n");
            printf("===============         WAIT         ===============\n");
            printf("====================================================\n");

            // Action ini dianggap tidak menghabiskan waktu untuk menghindari penambahan waktu ganda
            // Setelah penambahan waktu oleh command WAIT, tidak perlu lagi dilakukan penambahan waktu 1 menit
            validAction = false; 

            currentAct = currentWord;
            boolean allInteger = true, xint = false, yint = false;
            int waitHour, waitMinute, totalWaitMinute;

            // Cek terlebih dahulu apakah input valid (x dan y adalah integer)
            ADVWORD(command, &idx);
            if (isWordAllIntegers(currentWord)){
                waitHour = WordToInt(currentWord);
                xint = true;
            } else {
                allInteger = false;
            }
            
            ADVWORD(command, &idx);
            if (endWord){
                printf("Command wait membutuhkan 2 integer, X dan Y\n");
            }
            else {
                if (isWordAllIntegers(currentWord)){
                    waitMinute = WordToInt(currentWord);
                    yint = true;
                } else {
                    allInteger = false;
                }

                // cek lagi apakah terdapat integer ketiga
                ADVWORD(command, &idx);
                if (!endWord){
                    // ada integer (atau input lainnya) setelah integer kedua
                    printf("Command wait hanya membutuhkan tepat 2 integer, tidak lebih\n");
                }
                else {
                    if (allInteger){
                        if (waitHour == 0 && waitMinute == 0){
                            // Menunggu 0 0 boleh saja, tapi tidak melakukan perubahan apa apa
                            printf("Untuk apa dilakukan? Untuk apa? :(\n");
                        } else {
                            //Push ke Stack
                            currentState.sub3 = currentAct;
                            CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                            Push(&SUndo, currentState);
                            CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                            totalcommand ++;
                            totalundo=0;
                            printf("Menunggu ");
                            if (waitHour != 0) {
                                printf("%d jam ", waitHour);
                            }
                            if (waitMinute != 0){
                                printf("%d menit", waitMinute);
                            }
                            printf("\n");

                            totalWaitMinute = (waitHour * 60) + waitMinute;
                            currentState.sub2 = NextNMinute(currentState.sub2, totalWaitMinute);

                            decrementNExp(&(Inventory(currentState.sub1)), totalWaitMinute);
                            decrementNDel(&currentState.sub1.D, totalWaitMinute);
                            decrementNDel(&currentState.sub1.PL, totalWaitMinute);
                            
                            printf("Waktu pada Delivery List dan Inventory telah disesuaikan.\n");
                        }
                        
                    } else if (!yint) {
                        printf("Masukan tidak valid. Y bukan sebuah integer.\n");
                    } else if (!xint) {
                        printf("Masukan tidak valid. X bukan sebuah integer.\n");
                    } else {
                        printf("Masukan tidak valid. X dan Y hanya diperbolehkan memiliki tipe integer.\n");
                    }
                }
            }
        } 
        else if (isWordStringEqual(currentWord, "MOVE")){
            currentAct = currentWord;
            printf("====================================================\n");
            printf("===============         MOVE         ===============\n");
            printf("====================================================\n");

            while (!endWord){ 
                ADVWORD(command, &idx);
            }

            POINT src = Lokasi(currentState.sub1);
            ADVWORD(command, &idx);

            if (isWordStringEqual(currentWord, "NORTH")){
                //Push ke Stack
                currentState.sub3 = currentAct;
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                Push(&SUndo, currentState);
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                totalcommand ++;
                swapElmt(&map, &Lokasi(currentState.sub1), NextY(src));
            }
            else if (isWordStringEqual(currentWord, "EAST")){
                //Push ke Stack
                currentState.sub3 = currentAct;
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                Push(&SUndo, currentState);
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                totalcommand ++;
                swapElmt(&map, &Lokasi(currentState.sub1), NextX(src));
            } 
            else if (isWordStringEqual(currentWord, "SOUTH")){
                //Push ke Stack
                currentState.sub3 = currentAct;
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                Push(&SUndo, currentState);
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                totalcommand ++;
                swapElmt(&map, &Lokasi(currentState.sub1), BackY(src));
            }
            else if (isWordStringEqual(currentWord, "WEST")){
                //Push ke Stack
                currentState.sub3 = currentAct;
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                Push(&SUndo, currentState);
                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                totalcommand ++;
                swapElmt(&map, &Lokasi(currentState.sub1), BackX(src));
            } else {
                printf("Input selain 'NORTH', 'SOUTH', 'WEST', dan 'EAST' tidak diterima.\n");
                salahinput = true;
            }

            if (NEQ(src, Lokasi(currentState.sub1))){
                validAction = true;
                totalundo=0;
            }
            else{
                validAction = false;
                //Gajadi push ke Stack
                if (!salahinput){
                    Pop (&SUndo, &currentState);
                    totalcommand--;
                }
            }
        }
        else if (isWordStringEqual(currentWord, "FRIDGE")){
            printf("====================================================\n");
            printf("===============        FRIDGE        ===============\n");
            printf("====================================================\n");

            validAction = false; // Entah valid atau tidak, command yang berikatan dengan kulkas tidak menambah waktu
            while (!endWord){ 
                ADVWORD(command, &idx);
            }

            if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'K')){
                printf("Simulator tidak bersebelahan dengan tempat FRIDGE.\n");
                printf("Pastikan Simulator berada di sebelah petak 'K'\n");
                
            }
            else {
                if (isWordStringEqual(currentWord, "SHOW")){
                    // Melihat isi kulkas tidak membuang waktu
                    DisplayKulkas(k);
                }
                else if (isWordStringEqual(currentWord, "TAKE")){
                    if (isKulkasEmpty(k)){
                        printf("Kulkas kosong. Tidak ada yang bisa diambil dari kulkas.\n");

                    } else {
                        currentAct = currentWord;
                        DisplayKulkas(k);
                        printf("Masukkan petak yang untuk mengambil makanan dari kulkas: ");

                        fridgePoint = getKulkasCoordinate();
                        if (Absis(fridgePoint) == -1 || Ordinat(fridgePoint) == -1){
                            printf("Koordinat input yang dimasukkan tidaklah valid.\n");
                            
                        } else {
                            getMakananKulkas(&k, fridgePoint, &tempMkn);
                            EnqueueInventory(&Inventory(currentState.sub1), tempMkn);
                            //Push ke Stack
                            currentState.sub3 = currentAct;
                            CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                            Push(&SUndo, currentState);
                            totalcommand ++;
                            CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                        }
                    }
                } 
                else if (isWordStringEqual(currentWord, "PUT")){
                    //Push ke Stack
                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                    Push(&SUndo, currentState);
                    totalcommand ++;
                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                    if (IsEmptyQueue(Inventory(currentState.sub1))){
                        printf("Tidak ada makanan pada inventory. Tidak ada yang bisa dimasukkan pada kulkas.\n");
                    } else {
                        currentAct = currentWord;
                        DisplayKulkas(k);
                        DisplayInventory(currentState.sub1);
                        int invenLength = NBElmt(Inventory(currentState.sub1));
                        
                        printf("Masukkan makanan yang ingin dimasukkan : ");
                        getInput(command);
                        STARTWORD(command, &idx);
                        if (!isWordAllIntegers(currentWord)){
                            printf("Input bukanlah integer.\n");
                            
                        } else {
                            if (WordToInt(currentWord) < 1|| WordToInt(currentWord) > invenLength){
                            // Input integer, tapi tidak valid
                            printf("Input integer tidaklah valid.\n");
                            
                            } else {
                                int invenToKulkasIdx = WordToInt(currentWord) -1 ;
                                DequeueAtIndex(&Inventory(currentState.sub1), invenToKulkasIdx, &tempMkn);

                                printf("Masukkan petak yang untuk menaruh makanan pada kulkas: ");
                                fridgePoint = getKulkasCoordinate();
                                if (Absis(fridgePoint) == -1 || Ordinat(fridgePoint) == -1){
                                    printf("Koordinat input yang dimasukkan tidaklah valid.\n");
                                    EnqueueInventory(&Inventory(currentState.sub1), tempMkn);
                                    
                                } else {
                                    if (isPutAvailable(k, fridgePoint, tempMkn)){
                                        putMakananKulkas(&k, fridgePoint, tempMkn);


                                        //Push ke Stack
                                        currentState.sub3 = currentAct;
                                        CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                        Push(&SUndo, currentState);
                                        totalcommand ++;
                                        CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                            
                                    } else {
                                        printf("Makanan tidak bisa dimasukkan pada petak tersebut.\n");
                                        EnqueueInventory(&Inventory(currentState.sub1), tempMkn);
                                        
                                    }
                                }
                            }
                        }

                    }
                } else {
                    printf("Input selain 'SHOW', 'TAKE', dan 'PUT' tidak diterima.\n");
                    validAction = false; // Invalid Input
                }
            }
        }

        else {
            // bila bukan wait, maka cek lanjutannya
            ADVWORD(command, &idx);
            if (!endWord){
                // Handle untuk ada kata yang tidak diinginkan setelah command
                getInvalidRespond();
                validAction = false;
            }
            else {
                // bukan wait dan input sudah benar
                if (isWordStringEqual(currentWord, "EXIT")){
                    ADVWORD(command, &idx);
                    if (endWord){
                        printf("====================================================\n");
                        printf("===============        EXIT          ===============\n");
                        printf("====================================================\n");
                        printf("Yahh kok udahan... :(\n");
                        program = false;
                    } else {
                        getInvalidRespond();
                        validAction = false;
                    }
                }
                else if (isWordStringEqual(currentWord, "BUY")){
                    printf("====================================================\n");
                    printf("===============         BUY          ===============\n");
                    printf("====================================================\n");

                    // Program akan di loop pada sesi Buy
                    if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'T')){
                        printf("Simulator tidak bersebelahan dengan tempat melakukan BUY.\n");
                        printf("Pastikan Simulator berada di sebelah petak 'T'\n");
                        validAction = false;
                    } else {
                        currentAct = currentWord;
                        subprogram = true;
                        while (subprogram){
                        
                            count = countAndPrintAvailableFood(foodList, foodListLength, "BUY");
                        
                            // Meminta input dari pengguna
                            printf("Command: ");
                            getInput(command);
                            STARTWORD(command, &idx);
                            
                            // Handle untuk input tidak integer atau integer yang tidak valid
                            while (!isWordAllIntegers(currentWord) || WordToInt(currentWord) < 0 || WordToInt (currentWord) > count){
                                printf("Invalid input. Input bukanlah integer atau integer tersebut tidaklah valid.\n");
                                printf("Command: ");
                                getInput(command);
                                STARTWORD(command, &idx);
                            }
                            if (WordToInt(currentWord) == 0){
                                subprogram = false;
                                validAction = false; // Karena tidak melakukan apa-apa
                            } else {
                                //Push ke Stack
                                currentState.sub3 = currentAct;
                                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                Push(&SUndo, currentState);
                                totalcommand ++;
                                totalundo=0;
                                // Inputnya telah sesuai dengan penomoran 
                                // Mencari idx makanan pada list makanan sesuai penomoran input user
                                idxFood = searchIndexOlahMakanan(foodList, "BUY", WordToInt(currentWord));
                                EnqueueDelivery(&currentState.sub1.D, ELMTLIST(foodList, idxFood));
                                CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);

                                // Mengeluarkan pesan bahwa sudah dipesan.
                                printWord(nameMkn(ELMTLIST(foodList, idxFood)));
                                printf(" berhasil dipesan. Makanan akan diantar dalam ");
                                TulisTIMEString(dlvMkn(ELMTLIST(foodList, idxFood)));
                                printf("\n");
                                subprogram = false;
                            }
                        }
                    }

                }

                else if (isWordStringEqual(currentWord, "MIX")){
                    printf("====================================================\n");
                    printf("===============         MIX          ===============\n");
                    printf("====================================================\n");

                    // Program akan di loop pada sesi Mix
                    if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'M')){
                        printf("Simulator tidak bersebelahan dengan tempat melakukan MIX.\n");
                        printf("Pastikan Simulator berada di sebelah petak 'M'\n");
                        validAction = false;
                    } else {
                        currentAct = currentWord;
                        subprogram = true;
                        while (subprogram){
                        
                            count = countAndPrintAvailableFood(foodList, foodListLength, "MIX");
                        
                            // Meminta input dari pengguna
                            printf("Command: ");
                            getInput(command);
                            STARTWORD(command, &idx);
                            
                            // Handle untuk input tidak integer atau integer yang tidak valid
                            while (!isWordAllIntegers(currentWord) || WordToInt(currentWord) < 0 || WordToInt (currentWord) > count){
                                printf("Invalid input. Input bukanlah integer atau integer tersebut tidaklah valid.\n");
                                printf("Command: ");
                                getInput(command);
                                STARTWORD(command, &idx);
                            }
                            if (WordToInt(currentWord) == 0){
                                subprogram = false;
                                validAction = false; // Karena tidak melakukan apa-apa
                            } else {
                                // Inputnya telah sesuai dengan penomoran 
                                // Mencari idx makanan pada list makanan sesuai penomoran input user
                                idxFood = searchIndexOlahMakanan(foodList, "MIX", WordToInt(currentWord));
                                if (canMake(bukuResep, ELMTLIST(foodList, idxFood), Inventory(currentState.sub1))){
                                    int idPar, idxTree;
                                    for (int i = 0; i < NResep(bukuResep); i++){
                                        if (Parent(Resep(bukuResep, i)) == idMkn(ELMTLIST(foodList, idxFood))){
                                            idPar = Parent(Resep(bukuResep, i));
                                            idxTree = i;
                                        }
                                    }
                                    MixOlahInventory(&Inventory(currentState.sub1), &currentState.sub1.PL, bukuResep, idPar, idxTree, foodList);
                                    validAction = true;
                                    //Push ke Stack
                                    currentState.sub3 = currentAct;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                    Push(&SUndo, currentState);
                                    totalcommand ++;
                                    totalundo=0;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);

                                    //EnqueueDelivery(&currentState.sub1.PL, ELMTLIST(foodList, idxFood));

                                    // Mengeluarkan pesan bahwa sudah diproses.
                                    printWord(nameMkn(ELMTLIST(foodList, idxFood)));
                                    printf(" berhasil diproses. Makanan akan diproses dalam ");
                                    TulisTIMEString(dlvMkn(ELMTLIST(foodList, idxFood)));
                                    printf("\n");
                                    subprogram = false;
                                }
                                else {
                                    printf("Kamu tidak punya bahannya\n");
                                    printf("=========================\n");
                                    validAction = false;
                                }
                            }
                        }
                    }
                }

                else if (isWordStringEqual(currentWord, "CHOP")){
                    printf("====================================================\n");
                    printf("===============         CHOP         ===============\n");
                    printf("====================================================\n");

                    // Program akan di loop pada sesi Chop
                    if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'C')){
                        printf("Simulator tidak bersebelahan dengan tempat melakukan CHOP.\n");
                        printf("Pastikan Simulator berada di sebelah petak 'C'\n");
                        validAction = false;
                    } else {
                        currentAct = currentWord;
                        subprogram = true;
                        while (subprogram){
                        
                            count = countAndPrintAvailableFood(foodList, foodListLength, "CHOP");
                        
                            // Meminta input dari pengguna
                            printf("Command: ");
                            getInput(command);
                            STARTWORD(command, &idx);
                            
                            // Handle untuk input tidak integer atau integer yang tidak valid
                            while (!isWordAllIntegers(currentWord) || WordToInt(currentWord) < 0 || WordToInt (currentWord) > count){
                                printf("Invalid input. Input bukanlah integer atau integer tersebut tidaklah valid.\n");
                                printf("Command: ");
                                getInput(command);
                                STARTWORD(command, &idx);
                            }
                            if (WordToInt(currentWord) == 0){
                                subprogram = false;
                                validAction = false; // Karena tidak melakukan apa-apa
                            } else {
                                // Inputnya telah sesuai dengan penomoran 
                                // Mencari idx makanan pada list makanan sesuai penomoran input user  
                                idxFood = searchIndexOlahMakanan(foodList, "CHOP", WordToInt(currentWord));
                                if (canMake(bukuResep, ELMTLIST(foodList, idxFood), Inventory(currentState.sub1))){
                                    int idPar, idChld;
                                    for (int i = 0; i < NResep(bukuResep); i++){
                                        if (Parent(Resep(bukuResep, i)) == idMkn(ELMTLIST(foodList, idxFood))){
                                            idPar = Parent(Resep(bukuResep, i));
                                            idChld = Parent(Child(Resep(bukuResep, i), 0));
                                        }
                                    }
                                    ChopOlahInventory(&Inventory(currentState.sub1), &currentState.sub1.PL, getMakanan(idChld, foodList), getMakanan(idPar, foodList));
                                    validAction = true; 
                                    //Push ke Stack
                                    currentState.sub3 = currentAct;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                    Push(&SUndo, currentState);
                                    totalcommand ++;
                                    totalundo=0;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);

                                    // EnqueueDelivery(&currentState.sub1.PL, ELMTLIST(foodList, idxFood));

                                    // Mengeluarkan pesan bahwa sudah diproses.
                                    printWord(nameMkn(ELMTLIST(foodList, idxFood)));
                                    printf(" berhasil diproses. Makanan akan diproses dalam ");
                                    TulisTIMEString(dlvMkn(ELMTLIST(foodList, idxFood)));
                                    printf("\n");
                                    subprogram = false;
                                }
                                else {
                                    printf("Kamu tidak punya bahannya\n");
                                    printf("=========================\n");
                                    validAction = false; 
                                }
                            }
                        }
                    }
                }

                
                else if (isWordStringEqual(currentWord, "FRY")){
                    printf("====================================================\n");
                    printf("===============         FRY          ===============\n");
                    printf("====================================================\n");
                    
                    // Program akan di loop pada sesi Fry
                    if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'F')){
                        printf("Simulator tidak bersebelahan dengan tempat melakukan FRY.\n");
                        printf("Pastikan Simulator berada di sebelah petak 'F'\n");
                        validAction = false;
                    } else {
                        currentAct = currentWord;
                        subprogram = true;
                        while (subprogram){
                        
                            count = countAndPrintAvailableFood(foodList, foodListLength, "FRY");
                        
                            // Meminta input dari pengguna
                            printf("Command: ");
                            getInput(command);
                            STARTWORD(command, &idx);
                            
                            // Handle untuk input tidak integer atau integer yang tidak valid
                            while (!isWordAllIntegers(currentWord) || WordToInt(currentWord) < 0 || WordToInt (currentWord) > count){
                                printf("Invalid input. Input bukanlah integer atau integer tersebut tidaklah valid.\n");
                                printf("Command: ");
                                getInput(command);
                                STARTWORD(command, &idx);
                            }
                            if (WordToInt(currentWord) == 0){
                                subprogram = false;
                                validAction = false; // Karena tidak melakukan apa-apa
                            } else {
                                // Inputnya telah sesuai dengan penomoran 
                                // Mencari idx makanan pada list makanan sesuai penomoran input user
                                idxFood = searchIndexOlahMakanan(foodList, "FRY", WordToInt(currentWord));

                                if (canFry(Inventory(currentState.sub1)), canMake(bukuResep, ELMTLIST(foodList, idxFood), Inventory(currentState.sub1))){
                                    int idPar, idxTree;
                                    for (int i = 0; i < NResep(bukuResep); i++){
                                        if (Parent(Resep(bukuResep, i)) == idMkn(ELMTLIST(foodList, idxFood))){
                                            idPar = Parent(Resep(bukuResep, i));
                                            idxTree = i;
                                        }
                                    }
                                    FryOlahInventory(&Inventory(currentState.sub1), &currentState.sub1.PL, bukuResep, idPar, idxTree, foodList);
                                    validAction = true;
                                    //Push ke Stack
                                    currentState.sub3 = currentAct;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                    Push(&SUndo, currentState);
                                    totalcommand ++;
                                    totalundo=0;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);

                                    // EnqueueDelivery(&currentState.sub1.PL, ELMTLIST(foodList, idxFood));

                                    // Mengeluarkan pesan bahwa sudah diproses.
                                    printWord(nameMkn(ELMTLIST(foodList, idxFood)));
                                    printf(" berhasil diproses. Makanan akan diproses dalam ");
                                    TulisTIMEString(dlvMkn(ELMTLIST(foodList, idxFood)));
                                    printf("\n");
                                    subprogram = false;

                                
                                }
                                else {
                                    printf("Kamu tidak punya bahannya\n"); 
                                    printf("=========================\n");
                                    validAction = false;
                                }
                                
                            }
                        }
                    }
                }

                else if (isWordStringEqual(currentWord, "BOIL")){
                    printf("====================================================\n");
                    printf("===============         BOIL         ===============\n");
                    printf("====================================================\n");

                    // Program akan di loop pada sesi Chop
                    if (!isCan(map, Absis(Lokasi(currentState.sub1)), Ordinat(Lokasi(currentState.sub1)), 'B')){
                        printf("Simulator tidak bersebelahan dengan tempat melakukan BOIL.\n");
                        printf("Pastikan Simulator berada di sebelah petak 'B'\n");
                        validAction = false;
                    } else {
                        currentAct = currentWord;
                        subprogram = true;
                        while (subprogram){
                        
                            count = countAndPrintAvailableFood(foodList, foodListLength, "BOIL");
                        
                            // Meminta input dari pengguna
                            printf("Command: ");
                            getInput(command);
                            STARTWORD(command, &idx);
                            
                            // Handle untuk input tidak integer atau integer yang tidak valid
                            while (!isWordAllIntegers(currentWord) || WordToInt(currentWord) < 0 || WordToInt (currentWord) > count){
                                printf("Invalid input. Input bukanlah integer atau integer tersebut tidaklah valid.\n");
                                printf("Command: ");
                                getInput(command);
                                STARTWORD(command, &idx);
                            }
                            if (WordToInt(currentWord) == 0){
                                subprogram = false;
                                validAction = false; // Karena tidak melakukan apa-apa
                            } else {
                                // Inputnya telah sesuai dengan penomoran 
                                // Mencari idx makanan pada list makanan sesuai penomoran input user
                                idxFood = searchIndexOlahMakanan(foodList, "BOIL", WordToInt(currentWord));
                                if (canMake(bukuResep, ELMTLIST(foodList, idxFood), Inventory(currentState.sub1))){
                                    int idPar, idxTree;
                                    for (int i = 0; i < NResep(bukuResep); i++){
                                        if (Parent(Resep(bukuResep, i)) == idMkn(ELMTLIST(foodList, idxFood))){
                                            idPar = Parent(Resep(bukuResep, i));
                                            idxTree = i;
                                        }
                                    }
                                    BoilOlahInventory(&Inventory(currentState.sub1), &currentState.sub1.PL, bukuResep, idPar, idxTree, foodList);
                                    validAction = true;
                                    //Push ke Stack
                                    currentState.sub3 = currentAct;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                    Push(&SUndo, currentState);
                                    totalcommand ++;
                                    totalundo=0;
                                    CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                                    
                                    // EnqueueDelivery(&currentState.sub1.PL, ELMTLIST(foodList, idxFood));

                                    // Mengeluarkan pesan bahwa sudah diproses.
                                    printWord(nameMkn(ELMTLIST(foodList, idxFood)));
                                    printf(" berhasil diproses. Makanan akan diproses dalam ");
                                    TulisTIMEString(dlvMkn(ELMTLIST(foodList, idxFood)));
                                    printf("\n");
                                    subprogram = false;
                                }
                                else {
                                    printf("Kamu tidak punya bahannya\n");
                                    printf("=========================\n");
                                    validAction = false;
                                }
                            }
                        }
                    }
                }

                
                else if (isWordStringEqual(currentWord, "CATALOG")){
                    printf("====================================================\n");
                    printf("===============       CATALOG        ===============\n");
                    printf("====================================================\n");
                    validAction = false; // Action ini tidak menghabiskan waktu
                    //Note : Aku bikin fungsi baru printCatalog di makanan.c
                    printListCatalog(foodList, foodListLength);
                }

                else if (isWordStringEqual(currentWord, "COOKBOOK")){
                    printf("====================================================\n");
                    printf("===============       COOKBOOK       ===============\n");
                    printf("====================================================\n");
                    validAction = false; // Action ini tidak menghabiskan waktu
                    printf("List Resep");
                    printf("\n");
                    printf("(aksi yang diperlukan - bahan...)\n");
                    printResep(bukuResep, foodList);
                }

                else if (isWordStringEqual(currentWord, "INVENTORY")){
                    printf("====================================================\n");
                    printf("===============      INVENTORY       ===============\n");
                    printf("====================================================\n");
                    validAction = false; // Action ini tidak menghabiskan waktu

                    if (IsEmptyQueue(Inventory(currentState.sub1))){
                        printf("Tidak ada makanan pada inventory.\n");
                    } else {
                        DisplayInventory(currentState.sub1);
                    }
                }

                else if (isWordStringEqual(currentWord, "DELIVERY")){
                    printf("====================================================\n");
                    printf("===============       DELIVERY       ===============\n");
                    printf("====================================================\n");
                    validAction = false; // Action ini tidak menghabiskan waktu

                    if (IsEmptyQueue(currentState.sub1.D)){
                        printf("Tidak ada makanan pada list delivery.\n");
                    } else {
                        printf("List Makanan di Delivery List:\n");
                        printf("No - Nama - Waktu Sisa Delivery\n");
                        PrintPrioQueueTimeDelivery(currentState.sub1.D);
                    }
                }

                else if (isWordStringEqual(currentWord, "PROCESS")){
                    printf("====================================================\n");
                    printf("===============       PROCESS       ===============\n");
                    printf("====================================================\n");
                    validAction = false; // Action ini tidak menghabiskan waktu

                    if (IsEmptyQueue(currentState.sub1.PL)){
                        printf("Tidak ada makanan pada list process.\n");
                    } else {
                        printf("List Makanan di Process List:\n");
                        printf("No - Nama - Waktu Sisa Process\n");
                        PrintPrioQueueTimeProcess(currentState.sub1.PL);
                    }
                }

                else if (isWordStringEqual(currentWord, "UNDO")){
                    printf("====================================================\n");
                    printf("===============         UNDO         ===============\n");
                    printf("====================================================\n");
                    validAction = false;
                    if(totalcommand>0){
                        notifUndo = true;
                        undoAct = InfoTop(SUndo).sub3;
                        notifCount++;
                        POINT srcdummy;
                        CreatePoint(&srcdummy,-50,-50);
                        POINT lokasisekarang = Lokasi(currentState.sub1);
                        Undo(&SUndo,&SRedo,&currentState,totalcommand,srcdummy);
                        if (totalcommand>0){
                            totalcommand --;
                            totalundo++;
                        }
                        POINT lokasiundo = Lokasi(currentState.sub1);
                        Redo(&SUndo,&SRedo,&currentState,totalundo,srcdummy);
                        if (totalundo>0){
                            totalcommand++;
                            totalundo--;
                        }

                        POINT src = Lokasi(currentState.sub1);
                        if (lokasisekarang.X>lokasiundo .X){
                            swapElmt(&map, &Lokasi(currentState.sub1), BackX(src));
                        }
                        else if (lokasisekarang.X<lokasiundo .X){
                            swapElmt(&map, &Lokasi(currentState.sub1), NextX(src));
                        }
                        else if (lokasisekarang.Y>lokasiundo .Y){
                            swapElmt(&map, &Lokasi(currentState.sub1), NextY(src));
                        }
                        else if (lokasisekarang.Y<lokasiundo .Y){
                            swapElmt(&map, &Lokasi(currentState.sub1), BackY(src));
                        }
                        i = 0;
                        CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                        Undo(&SUndo,&SRedo,&currentState,totalcommand,src);
                        if (totalcommand>0){
                            totalcommand --;
                            totalundo++;
                        }
                        printf("Undo telah dilakukan\n");
                    }
                    else {
                        printf("Undo tidak bisa dilakukan\n");
                    }
                }

                else if (isWordStringEqual(currentWord, "REDO")){
                    printf("====================================================\n");
                    printf("===============         REDO         ===============\n");
                    printf("====================================================\n");

                    validAction = false;
                    if (totalundo>0){
                        notifRedo = true;
                        redoAct = InfoTop(SRedo).sub3;
                        notifCount++;
                        POINT srcdummy;
                        CreatePoint(&srcdummy,-50,-50);
                        POINT lokasisekarang = Lokasi(currentState.sub1);
                        Redo(&SUndo,&SRedo,&currentState,totalundo,srcdummy);
                        if (totalundo>0){
                            totalcommand++;
                            totalundo--;

                        }
                        POINT lokasiredo = Lokasi(currentState.sub1);
                        Undo(&SUndo,&SRedo,&currentState,totalcommand,srcdummy);
                        if (totalcommand>0){
                            totalcommand --;
                            totalundo++;
                        }
                        POINT src = Lokasi(currentState.sub1);
                        if (lokasisekarang.X>lokasiredo.X){
                            swapElmt(&map, &Lokasi(currentState.sub1), BackX(src));
                        }
                        else if (lokasisekarang.X<lokasiredo .X){
                            swapElmt(&map, &Lokasi(currentState.sub1), NextX(src));
                        }
                        else if (lokasisekarang.Y>lokasiredo .Y){
                            swapElmt(&map, &Lokasi(currentState.sub1), NextY(src));
                        }
                        else if (lokasisekarang.Y<lokasiredo .Y){
                            swapElmt(&map, &Lokasi(currentState.sub1), BackY(src));
                        }
                        CreateSimulatorUndo(&currentState.sub1,currentState.sub1.Nama,currentState.sub1.P,currentState.sub1.Q,currentState.sub1.D,currentState.sub1.PL);
                        Redo(&SUndo,&SRedo,&currentState,totalundo,src);
                        if (totalundo>0){
                            totalcommand++;
                            totalundo--;
                            jumlahredo++;
                        }
                        printf("Redo telah dilakukan\n");
                    }
                    else {
                        printf("Redo tidak bisa dilakukan\n");
                        for (idxredo;idxredo<jumlahredo;idxredo++){
                            dummyredo = InfoTop(SRedo);
                            Top(SRedo)--;
                        }
                    }
                }

                else if (isWordStringEqual(currentWord, "HELP")){
                    getHelp();
                    validAction = false; // Action ini tidak menghabiskan waktu
                }
                else {
                    getInvalidRespond();
                    validAction = false;
                }

            }
        }

        // ================= AFTER ACTION ALGORITHM =================

        if (validAction){
            // Waktu hanya ditambahkan bila action yang dilakukan valid
            currentState.sub2 = NextMinute(currentState.sub2);
            
            // Mengurangi waktu di delivery list, process list dan inventory
            decrementNDel(&currentState.sub1.D, 1);
            decrementNDel(&currentState.sub1.PL, 1);
            decrementNExp(&Inventory(currentState.sub1), 1);
        }

        // Tetap bisa dilakukan meski ValidAction = false (contohnya menggunakan WAIT)
        // Mengeluarkan dari Delivery List, memasukan ke inventory (bila sampai)
        if (!IsEmptyQueue(currentState.sub1.D)){
            while ((!TGT(dlvMkn(InfoHead(currentState.sub1.D)), boundariesTime)) && (NBElmt(currentState.sub1.D) > 1)){
                Dequeue(&currentState.sub1.D, &dumpMkn);
                // Mengurangi waktu kadaluarsa sesuai dengan sisa pada delivery
                int remainder = TIMEtoint(dlvMkn(dumpMkn));
                TIME newExpiry = inttoTIME(TIMEtoint(expMkn(dumpMkn)) + remainder);
                expMkn(dumpMkn) = newExpiry;
                EnqueueInventory(&Inventory(currentState.sub1), dumpMkn);
                notifCount++;
                deliveredCount++;
                insertLast(&notifList, dumpMkn);
            }
            if (!TGT(dlvMkn(InfoHead(currentState.sub1.D)), boundariesTime)){
                Dequeue(&currentState.sub1.D, &dumpMkn);
                // Mengurangi waktu kadaluarsa sesuai dengan sisa pada delivery
                int remainder = TIMEtoint(dlvMkn(dumpMkn));
                TIME newExpiry = inttoTIME(TIMEtoint(expMkn(dumpMkn)) + remainder);
                expMkn(dumpMkn) = newExpiry;
                EnqueueInventory(&Inventory(currentState.sub1), dumpMkn);
                notifCount++;
                deliveredCount++;
                insertLast(&notifList, dumpMkn);
            }
        }

        // Mengeluarkan dari Process List, memasukan ke inventory (bila selesai process)
        if (!IsEmptyQueue(currentState.sub1.PL)){
            while ((!TGT(dlvMkn(InfoHead(currentState.sub1.PL)), boundariesTime)) && (NBElmt(currentState.sub1.PL) > 1)){
                Dequeue(&currentState.sub1.PL, &dumpMkn);
                // Mengurangi waktu kadaluarsa sesuai dengan sisa pada process time
                int remainder = TIMEtoint(dlvMkn(dumpMkn));
                TIME newExpiry = inttoTIME(TIMEtoint(expMkn(dumpMkn)) + remainder);
                expMkn(dumpMkn) = newExpiry;
                EnqueueInventory(&Inventory(currentState.sub1), dumpMkn);
                notifCount++;
                processedCount++;
                insertLast(&notifList, dumpMkn);
            }
            if (!TGT(dlvMkn(InfoHead(currentState.sub1.PL)), boundariesTime)){
                Dequeue(&currentState.sub1.PL, &dumpMkn);
                // Mengurangi waktu kadaluarsa sesuai dengan sisa pada process time
                int remainder = TIMEtoint(dlvMkn(dumpMkn));
                TIME newExpiry = inttoTIME(TIMEtoint(expMkn(dumpMkn)) + remainder);
                expMkn(dumpMkn) = newExpiry;
                EnqueueInventory(&Inventory(currentState.sub1), dumpMkn);
                notifCount++;
                processedCount++;
                insertLast(&notifList, dumpMkn);
            }
        }

        // Mengeluarkan dari inventory ( bila kadaluarsa)
        if (!IsEmptyQueue(Inventory(currentState.sub1))){
            // Menghapus sampai bersisa 1
            while ((!TGT(expMkn(InfoHead(Inventory(currentState.sub1))), boundariesTime)) && (NBElmt(Inventory(currentState.sub1)) > 1)){
                Dequeue(&Inventory(currentState.sub1), &dumpMkn);
                notifCount++;
                expiredCount++;
                insertLast(&notifList, dumpMkn);
            }
            // Menghapus makanan terakhir bila bersisa 1
            if (!TGT(expMkn(InfoHead(Inventory(currentState.sub1))), boundariesTime)){
                Dequeue(&Inventory(currentState.sub1), &dumpMkn);
                notifCount++;
                expiredCount++;
                insertLast(&notifList, dumpMkn);
            }
        }
    }
    return 0;
}