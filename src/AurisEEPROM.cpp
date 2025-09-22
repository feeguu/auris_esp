#include <AurisEEPROM.h>
#include <cstring>
#include <Arduino.h>

void EEPROMInit()
{
  Serial.printf("Tamanho da estrutura AurisConfig: %d bytes\n", sizeof(AurisConfig));
  Serial.printf("Tamanho da EEPROM: %d bytes\n", EEPROM_SIZE);

  EEPROM.begin(EEPROM_SIZE);

  // Check sentinel value
  uint16_t sentinel = 0;
  EEPROM.get(0, sentinel);
  if (sentinel != EEPROM_SENTINEL_VALUE)
  {
    // Initialize EEPROM with default values
    Serial.println("Inicializando EEPROM com valores padrão...");
    sentinel = EEPROM_SENTINEL_VALUE;

    EEPROM.put(0, sentinel);

    AurisConfig defaultConfig = {};
    strcpy(defaultConfig.ssid, "AURIS");
    strcpy(defaultConfig.password, "AURIS123");
    strcpy(defaultConfig.language, "pt");
    defaultConfig.font_size = 16;
    strcpy(defaultConfig.version, "1.0.0");
    strcpy(defaultConfig.firmware_url, "https://github.com/feeguu/auris_esp/releases/latest/download/firmware.bin");
    defaultConfig.captions_enabled = false;

    // Write using individual field approach
    int addr = EEPROM_START_ADDRESS;

    // Write SSID
    for (int i = 0; i < MAX_SSID_LENGTH; i++)
    {
      EEPROM.write(addr++, defaultConfig.ssid[i]);
    }

    // Write password
    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++)
    {
      EEPROM.write(addr++, defaultConfig.password[i]);
    }

    // Write language
    for (int i = 0; i < MAX_LANGUAGE_LENGTH; i++)
    {
      EEPROM.write(addr++, defaultConfig.language[i]);
    }

    // Write font_size
    EEPROM.write(addr++, defaultConfig.font_size);

    // Write version
    for (int i = 0; i < MAX_VERSION_LENGTH; i++)
    {
      EEPROM.write(addr++, defaultConfig.version[i]);
    }

    // Write firmware_url
    for (int i = 0; i < MAX_URL_LENGTH; i++)
    {
      EEPROM.write(addr++, defaultConfig.firmware_url[i]);
    }

    // Write captions_enabled
    EEPROM.write(addr++, defaultConfig.captions_enabled);

    EEPROM.commit();
    Serial.println("EEPROM inicializada com sucesso.");
  }
  else
  {
    Serial.println("EEPROM já inicializada.");
  }
}

bool EEPROMReadConfig(AurisConfig &config)
{
  // Initialize the entire structure with zeros first
  memset(&config, 0, sizeof(AurisConfig));

  uint16_t sentinel = 0;
  EEPROM.get(0, sentinel);
  Serial.printf("Sentinel lido: 0x%04X (esperado: 0x%04X)\n", sentinel, EEPROM_SENTINEL_VALUE);

  if (sentinel != EEPROM_SENTINEL_VALUE)
  {
    // Invalid sentinel, no valid config - set defaults
    Serial.println("Configuração inválida, definindo padrões...");
    strcpy(config.ssid, "AURIS");
    strcpy(config.password, "AURIS123");
    strcpy(config.language, "pt");
    config.font_size = 16;
    strcpy(config.version, "1.0.0");
    strcpy(config.firmware_url, "http://auris.com.br/firmware");
    config.captions_enabled = false;
    return true; // Return true since we've set defaults
  }

  Serial.println("Configuração válida encontrada, carregando...");

  // Read each field individually to avoid struct alignment issues
  int addr = EEPROM_START_ADDRESS;

  // Read SSID
  for (int i = 0; i < MAX_SSID_LENGTH; i++)
  {
    config.ssid[i] = EEPROM.read(addr++);
  }

  // Read password
  for (int i = 0; i < MAX_PASSWORD_LENGTH; i++)
  {
    config.password[i] = EEPROM.read(addr++);
  }

  // Read language
  for (int i = 0; i < MAX_LANGUAGE_LENGTH; i++)
  {
    config.language[i] = EEPROM.read(addr++);
  }

  // Read font_size
  config.font_size = EEPROM.read(addr++);

  // Read version
  for (int i = 0; i < MAX_VERSION_LENGTH; i++)
  {
    config.version[i] = EEPROM.read(addr++);
  }

  // Read firmware_url
  for (int i = 0; i < MAX_URL_LENGTH; i++)
  {
    config.firmware_url[i] = EEPROM.read(addr++);
  }

  // Read captions_enabled
  config.captions_enabled = EEPROM.read(addr++);

  return true;
}

bool EEPROMWriteConfig(const AurisConfig &config)
{
  uint16_t sentinel = EEPROM_SENTINEL_VALUE;
  EEPROM.put(0, sentinel);

  // Write each field individually to avoid struct alignment issues
  int addr = EEPROM_START_ADDRESS;

  // Write SSID
  for (int i = 0; i < MAX_SSID_LENGTH; i++)
  {
    EEPROM.write(addr++, config.ssid[i]);
  }

  // Write password
  for (int i = 0; i < MAX_PASSWORD_LENGTH; i++)
  {
    EEPROM.write(addr++, config.password[i]);
  }

  // Write language
  for (int i = 0; i < MAX_LANGUAGE_LENGTH; i++)
  {
    EEPROM.write(addr++, config.language[i]);
  }

  // Write font_size
  EEPROM.write(addr++, config.font_size);

  // Write version
  for (int i = 0; i < MAX_VERSION_LENGTH; i++)
  {
    EEPROM.write(addr++, config.version[i]);
  }

  // Write firmware_url
  for (int i = 0; i < MAX_URL_LENGTH; i++)
  {
    EEPROM.write(addr++, config.firmware_url[i]);
  }

  // Write captions_enabled
  EEPROM.write(addr++, config.captions_enabled);

  return EEPROM.commit();
}
