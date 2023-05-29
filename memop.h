#ifndef __memop__h__
#define __memop__h__

#include "FS.h"
#include <LittleFS.h>
#include "telnet.h"

String listDir(fs::FS &fs, const char * dirname) {
  String str;
  str+=String("Listing directory: ")+dirname+"\n";
  File root = fs.open(dirname);
  if(!root){
      str+=String("- failed to open directory\n");
      return str;
  }
  if(!root.isDirectory()){
      str+=String(" - not a directory\n");
      return str;
  }

  File file = root.openNextFile();
  while(file){
      if(file.isDirectory()){
          str+=String("  DIR : ")+file.name()+"\n";
          str+=listDir(fs, file.path());
      } else {
          str+=String("  FILE: ")+file.name()+"\tSIZE: "+String(file.size());
      }
      file = root.openNextFile();
  }
  return str;
}
void SetupFS() {
  // Check if FS is ok, ohterwise format partition
  if (!LittleFS.begin()) {
    Log.print("LittleFS needs formatting T=" + String(millis())+"\n");
    if(LittleFS.format()) {
      Log.print("File System formatted T=" + String(millis())+"\n");
    } else {
      Log.print("File System formatting Error T=" + String(millis())+"\n");
    }
  } else {
    Log.print("Done! T=" + String(millis()));
    Log.print(" Size=" + String(LittleFS.totalBytes()));
    Log.print(" Used=" + String(LittleFS.usedBytes())+"\n");
    LittleFS.end();
  }
}
// Verifica se tem arquivo de memirrig.dat
// Se tem, lê e coloca em mem
// senão, inicializa mem como antes

// Verifica se tem arquivo de minimax.dat
// Se tem, sobrescreve o minimax com o arquivo e depois deleta o arquivo

// verifica se tem o arquivo datalogger.dat
// se tem, lê o struct, monta o Json e envia até acabar, usa o último para colocar nos structs de valores correntes, depois deleta o arquivo

// Tenta salvar a wifi usando o wifimanager e uma função para salvar, ver exemplos
// Se tem arquivo wifi.dat
// Lê e tenta logar, se não conseguir, inicia o wifimaneger normalmente, 
/*
void InitSPIFFS() {
  FSInfo fs_info;

  Log.print("SPIFFS check T=" + String(millis())+"\n");
  SPIFFS.begin();

  if (!SPIFFS.info(fs_info)) {
    Log.print("SPIFFS needs formatting T=" + String(millis())+"\n");
    if(SPIFFS.format()) {
      Log.print("File System formatted T=" + String(millis())+"\n");
    } else {
      Log.print("File System formatting Error T=" + String(millis())+"\n");
    }
  } else {
    Log.print("Done! T=" + String(millis()));
    Log.print(" Size=" + String(fs_info.totalBytes));
    Log.print(" Used=" + String(fs_info.usedBytes)+"\n");

    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      Log.print(dir.fileName());
        File f = dir.openFile("r");
        Log.print("  " + String(f.size())+"\n");
        f.close();
    }
  }

  SPIFFS.end();
}
*/

/***************************************************************************
 * Load configuration data from the SPIFF file, return 0 if OK
 * SPIFFS open function needs path and mode:
 * mode = "r", "w", "a", "r+", "w+", "a+" (text file) or "rb", "wb", "ab", "rb+", "wb+", "ab+" (binary)
 * where r = read, w = write, a = append
 * + means file is open for update (read and write)
 * b means the file os open for binary operations
 * 
 * Returns 0 if OK else:
 * -1 = No SPIFFS file system
 * -2 = File does not exist
 * -3 = File too short
 * -4 = Checksum does not compare
 ***************************************************************************/
 /*
int LoadConfigFromFile(char * configfile)
{
  FSInfo fs_info;
  File F;
  int retcode = 0;
  unsigned int readSize;
  unsigned int chk;
  
  SPIFFS.begin();
  
  if (!SPIFFS.info(fs_info)) 
    retcode = -1; //File system not initialized?
  else
  {
    if (!SPIFFS.exists(configfile)) 
      retcode = -2; //Could not find config file
    else
    {
      F = SPIFFS.open(configfile, "rb");
      F.setTimeout(0);
      readSize = F.readBytes((char*) mem, sizeof(Cnf)); //cast changed from byte*
      if (!(readSize == sizeof(Cnf)))
        retcode = -3;
      else
      {
        chk = Cnf.checksum;
        Cnf.checksum = 0;
        if (CheckSum(&Cnf, sizeof(Cnf)) == chk) //Validated checksum
        {
          Cnf.checksum = chk;
          *Conf = Cnf;
        }
        else
          retcode = -4;
      }
      F.close();  
    }
  }
  SPIFFS.end();
  
  return retcode;
}
*/

/*********************************************************************
 * Write configuration into a SPIFF file at address 0
 * SPIFFS open function needs path and mode:
 * mode = "r", "w", "a", "r+", "w+", "a+" (text file) or "rb", "wb", "ab", "rb+", "wb+", "ab+" (binary)
 * where r = read, w = write, a = append
 * + means file is open for update (read and write)
 * b means the file os open for binary operations
 * 
 * Returns 0 if OK else:
 * -1 = No SPIFFS file system
 * -3 = All bytes could not be written
 *********************************************************************/
 /*
int WriteConfigToFile(char* configfile)
{
  FSInfo fs_info;
  File F;
  unsigned int writeSize;
  int retcode = 0;
  
  UpdateChecksum(&mem);  //Set checksum member to sum of all other bytes

  SPIFFS.begin();
  
  if (!SPIFFS.info(fs_info)) 
    retcode = -1; //File system not initialized?
  else
  {
    F = SPIFFS.open(configfile, "wb");
    writeSize = F.write((byte*) Cnf, sizeof(Cnf));
    if (!(writeSize == sizeof(Cnf)))
      retcode = -3;
    F.close();  
  }
  SPIFFS.end();
  
  return retcode;
}

void setupSpiffs() {
  // Retaura o conteudo da memória, se existir
  // se não existir, coloca valores padrão
  InitSPIFFS();
  
}
*/
#endif
