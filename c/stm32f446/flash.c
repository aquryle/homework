/*
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K
  FLASH    (rx)    : ORIGIN = 0x8000000,   LENGTH = 384K
  DATA     (rx)    : ORIGIN = 0x8060000,   LENGTH = 128K
}
*/

int flash_erase(int sector)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = sector;  // 例: FLASH_SECTOR_5
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
    {
        // エラー処理
        Error_Handler();
    }

    HAL_FLASH_Lock();

    return 0;
}

int flash_write(uint32_t addr, uint32_t size, uint8_t *data)
{
    HAL_FLASH_Unlock();

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *data) != HAL_OK)
    {
        // エラー処理
        Error_Handler();
    }

    HAL_FLASH_Lock();

    return 0;
}

uint32_t flash_read(uint32_t addr)
{
	return *(__IO uint32_t*)addr;
}
