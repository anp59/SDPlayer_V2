//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32,                                                              *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 3  , Jul.02/2020
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

// Digital I/O used
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define MAX98357A_SD 22

Audio audio;
// WiFiMulti wifiMulti;
//String ssid = "xxxxx";
//String password = "xxxxx";

// for directory listing over Serial (optional)
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
const char *name(file_t &f);

void setup()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    pinMode(MAX98357A_SD, OUTPUT);
    digitalWrite(MAX98357A_SD, LOW); // mute

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(25000000); // 25 MHz
    Serial.begin(115200);
    SD.begin(SD_CS);
    // WiFi.mode(WIFI_STA);
    // wifiMulti.addAP(ssid.c_str(), password.c_str());
    // wifiMulti.run();
    // if (WiFi.status() != WL_CONNECTED)
    // {
    //     WiFi.disconnect(true);
    //     wifiMulti.run();
    // }
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.forceMono(true);
    audio.setTone(5, 0, 3);
    audio.setVolume(12); // 0...21
    
    digitalWrite(MAX98357A_SD, HIGH); // MAX98357A SD-Mode left channel
    
    listDir(SD, "/", 10);

    // audio.connecttoFS(SD, "test.mp3");
    audio.connecttoFS(SD, "/Musik/Gretel/01 La mamma morta.mp3"); // ESP32: Guru Meditation Error: Core  1 panic'ed (LoadStoreError). Exception was unhandled.
    // audio.connecttoFS(SD, "/Musik/Gretel/01 Sempre libera.mp3"); // ESP32: Guru Meditation Error: Core  1 panic'ed (LoadStoreError). Exception was unhandled.
    // audio.connecttoFS(SD, "/Musik/Wiener Philharmoniker - New Year's Concert 2015/CD2/05. Annen-Polka, op.117.mp3");
    // audio.connecttoFS(SD, "/Musik/Wiener Philharmoniker - New Year's Concert 2015/CD2/10. An der schönen blauen Donau, op.314.mp3");
    // audio.connecttoFS(SD, "/Musik/André Rieu/Bal Du Siècle/02 España Cañi.mp3"); // file can't open because of incomplete character conversion of 'é')
    //    audio.connecttoFS(SD, "test.wav");
    
    //    audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
    //    audio.connecttohost("http://macslons-irish-pub-radio.com/media.asx");
    //    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.aac"); //  128k aac
    //    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
}

void loop()
{
    audio.loop();
    if (Serial.available())
    { // put streamURL in serial monitor
        audio.stopSong();
        String r = Serial.readString();
        r.trim();
        if (r.length() > 5)
            audio.connecttohost(r.c_str());
        log_i("free heap=%i", ESP.getFreeHeap());
    }
}

// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}

// ###############################################################
//  optional functions for listing directory

const char *name(file_t &f)
{
#ifdef SDFATFS_USED
    static char buf[256];
    buf[0] = 0;
    if (f)
        f.getName(buf, sizeof(buf));
    return (const char *)buf;
#else
    return f.name();
#endif
}

#ifdef SDFATFS_USED
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
    {
        char path[256] = ""; // muss mit 0 initialisiert sein
        int len = 0;         // muss für SD-Lib mit 0 initialisiert sein
        file_t root;
        int mode = 2;

        root.open(dirname);
        if (!root)
        {
            Serial.println("Failed to open directory");
            return;
        }
        if (!root.isDir())
        {
            Serial.println("Not a directory");
            return;
        }
        Serial.println("----------------------------------------------");
        Serial.printf("Listing directory: %s (I%d)\n", dirname, root.dirIndex());

        while (true)
        {
            file_t file;
            file.openNext(&root, O_RDONLY);
            while (file)
            {
                if (file.isDir() && mode == 1)
                {
                    if (file.isHidden())
                        Serial.print("*");
                    Serial.print("DIR : ");
                    Serial.printf("%s (L%d - I%d)\n", name(file), levels, file.dirIndex());
                    if (levels)
                    {
                        // nur bei SdFat - kompletten pfad für file rekursiv weitergeben
                        if ((name(file))[0] != '/')
                        {
                            strcpy(path, dirname);
                            len = strlen(path);
                            if (!(len == 1 && path[0] == '/')) // not root (/)
                                path[len++] = '/';             // ohne abschliessende 0
                        }
                        strcpy(path + len, name(file));
                        listDir(fs, path, levels - 1);
                    }
                }

                if (!file.isDir() && mode == 2)
                {
                    if (file.isHidden())
                        Serial.print("*");
                    Serial.print("  FILE: ");
                    Serial.printf("%s (L%d - I%d)\n", name(file), levels, file.dirIndex());
                    // Serial.print("  SIZE: ");
                    // Serial.println(file.size());   
                }             
                file.close();
                file.openNext(&root, O_RDONLY);
            }
            mode--;
            if (!mode)
            {
                root.close();
                break;
            }

            if (root.isDir())
                root.rewind();
        }
    }
#else
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
    {
        char path[256] = ""; // muss mit 0 initialisiert sein
        int len = 0;         // muss für SD-Lib mit 0 initialisiert sein
        file_t root;
        int mode = 2;

        root = fs.open(dirname);
        if (!root)
        {
            Serial.println("Failed to open directory");
            return;
        }
        if (!root.isDirectory())
        {
            Serial.println("Not a directory");
            return;
        }
        Serial.println("----------------------------------------------");
        Serial.printf("Listing directory: %s\n", dirname);
        while (true)
        {
            file_t file = root.openNextFile();
            while (file)
            {
                if (file.isDirectory() && mode == 1)
                {
                    Serial.print("DIR : ");
                    Serial.printf("%s (L%d)\n", name(file), levels);
                    if (levels)
                    {
                        // nur bei SdFat - kompletten pfad für file rekursiv weitergeben
                        if ((name(file))[0] != '/')
                        {
                            strcpy(path, dirname);
                            len = strlen(path);
                            if (!(len == 1 && path[0] == '/')) // not root (/)
                                path[len++] = '/';             // ohne abschliessende 0
                        }
                        strcpy(path + len, name(file));
                        listDir(fs, path, levels - 1);
                    }
                }
                if (!file.isDirectory() && mode == 2)
                {
                    Serial.print("  FILE: ");
                    Serial.printf("%s (L%d)\n", name(file), levels);
                }
                file.close();
                file = root.openNextFile();
            }
            mode--;
            if (!mode)
            {
                root.close();
                break;
            }
            root.rewindDirectory();
        }
    }
#endif // #ifdef SDFATFS_USED