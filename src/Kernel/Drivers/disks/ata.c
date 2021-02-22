#include "BalrogOS/Drivers/disk/ata.h"
#include "BalrogOS/Drivers/disk/ata_command.h"
#include "BalrogOS/CPU/Ports/ports.h"

#include "BalrogOS/Debug/debug_output.h"

ata_drive drives[4];

static inline void _ata_fill_buffer(uint16_t io_bus, uint16_t* buf)
{
    for(size_t i = 0; i < 256; i++)
    {
        buf[i] = in_word(ATA_REG_R_DATA(io_bus));
    }
}

static int _ata_wait_400ns(uint16_t bus)
{
    in_byte(ATA_REG_R_STATUS(bus));
    in_byte(ATA_REG_R_STATUS(bus));
    in_byte(ATA_REG_R_STATUS(bus));
    in_byte(ATA_REG_R_STATUS(bus));
    return in_byte(ATA_REG_R_STATUS(bus));
}

static uint8_t _ata_send_command(ata_cmd* command)
{
    out_byte(ATA_REG_W_DEV(command->bus), command->device | (command->lba_sep[3] &0xf));
    _ata_wait_400ns(command->bus);

    out_byte(ATA_REG_W_S_COUNT(command->bus), command->count);
    out_byte(ATA_REG_W_LBA_L(command->bus), command->lba_sep[0]);
    out_byte(ATA_REG_W_LBA_M(command->bus), command->lba_sep[1]);
    out_byte(ATA_REG_W_LBA_H(command->bus), command->lba_sep[2]);

    while(in_byte(ATA_REG_R_STATUS(command->bus)) & ATA_STATUS_BSY)
    {}

    out_byte(ATA_REG_W_CMD(command->bus), command->command);

    if(!in_byte(ATA_REG_R_STATUS(command->bus)))
    {
        return 0;
    }

    command->status = _ata_wait_400ns(command->bus);

    while((command->status = in_byte(ATA_REG_R_STATUS(command->bus))) & ATA_STATUS_BSY)
    {}

    if(command->wait_status)
    {
        while(!(command->status = in_byte(ATA_REG_R_STATUS(command->bus)) & (command->wait_status | ATA_STATUS_ERR)))
        {}
    }

    command->error = in_byte(ATA_REG_R_ERROR(command->bus));
    command->count = in_byte(ATA_REG_R_S_COUNT(command->bus));
    command->device = in_byte(ATA_REG_R_DEV(command->bus));
    command->lba_sep[0] = in_byte(ATA_REG_R_LBA_L(command->bus));
    command->lba_sep[1] = in_byte(ATA_REG_R_LBA_M(command->bus));
    command->lba_sep[2] = in_byte(ATA_REG_R_LBA_H(command->bus));
    command->lba_sep[3] = 0;

    return command->status;
}

static void _ata_init_drive(ata_drive* drive)
{
    uint16_t io_bus = drive->io_bus;

    out_byte(ATA_REG_W_S_COUNT(io_bus), 0);
    out_byte(ATA_REG_W_LBA_L(io_bus), 0);
    out_byte(ATA_REG_W_LBA_M(io_bus), 0);
    out_byte(ATA_REG_W_LBA_H(io_bus), 0);

    uint8_t val_1 = in_byte(ATA_REG_R_LBA_L(io_bus));
    out_byte(ATA_REG_W_LBA_L(io_bus), (~val_1) & 0xff);
    uint8_t val_2 = in_byte(ATA_REG_R_LBA_L(io_bus));
    if(val_2 != ((~val_1) & 0xff))
    {
        KERNEL_LOG_FAIL("fail to read drive 0%x | %s", io_bus, drive->master == 0x10 ? "slave" : "master");
        return;
    }

    out_byte(ATA_REG_W_DEV(io_bus), 0xa0 | drive->master);
    if(!(_ata_wait_400ns(io_bus) & ATA_STATUS_RDY))
    {
        KERNEL_LOG_FAIL("drive is not ready! 0%x | %s", io_bus, drive->master == 0x10 ? "slave" : "master");
        return;
    }

    // do a software rest
    out_byte(ATA_REG_W_CTR(drive->ctr_bus), ATA_CTR_SRST);
    // clear the reset bit
    out_byte(ATA_REG_W_CTR(drive->ctr_bus), 0);

    // get logical block address
    uint64_t lba = (in_byte(ATA_REG_R_LBA_H(io_bus)) << 16) | (in_byte(ATA_REG_R_LBA_H(io_bus)) << 8) | in_byte(ATA_REG_R_LBA_L(io_bus));

    ata_cmd command;
    command.bus = io_bus;
    command.device = 0xa0 | drive->master;
    command.command = ATA_CMD_IDENT_DEV;
    command.wait_status = ATA_STATUS_DRQ;
    if(lba == ATAPI_LBA_MAGIC)
    {
        KERNEL_LOG_OK("drive is ATAPI, 0%x", io_bus);
        drive->atapi = 1;

    }

    if(!_ata_send_command(&command))
    {
        KERNEL_LOG_FAIL("fail to exectue command");
        return;
    }

    _ata_fill_buffer(io_bus, (void*) &drive->id);

    KERNEL_LOG_OK("drive 0%x %s load successfully.", io_bus, drive->master == 0x10 ? "slave" : "master");

    drive->exist = 1;
}

static inline int _ata_read_sector(uint8_t* buffer, uint64_t lba, ata_drive* device)
{
    int retries = 5;
    while(retries--)
    {
        ata_cmd command;
        command.bus = device->io_bus;
        command.count = 1;
        command.lba = lba & 0xffffff;
        command.device = 0xe0 | device->master | ((lba >> 24) & 0xf);
        command.command = ATA_CMD_READ_SEC_RETRY;

        int status = _ata_send_command(&command);
        if(status & (ATA_STATUS_DF | ATA_STATUS_ERR) || !(status & ATA_STATUS_DRQ))
        {
            continue;
        }

        _ata_fill_buffer(device->io_bus, buffer);
        return 0;
    }
    return -1;
}

void ata_read(uint8_t* buffer, uint64_t lba, uint64_t len, ata_drive* device)
{

}

static inline void _ata_write_sector(uint8_t* buffer, uint64_t lba, ata_drive* device)
{

}

void ata_write(uint8_t* buffer, uint64_t lba, uint64_t len, ata_drive* device)
{

}

uint16_t buffer[256];

void init_ata()
{
    drives[0].io_bus = ATA_DEV_IO_PREMARY;
    drives[0].ctr_bus = ATA_DEV_CTR_PRIMARY;
    drives[0].exist = 0;
    drives[0].master = ATA_MASTER;
    _ata_init_drive(&drives[0]);

    drives[1].io_bus = ATA_DEV_IO_PREMARY;
    drives[1].ctr_bus = ATA_DEV_CTR_PRIMARY;
    drives[1].exist = 0;
    drives[1].master = ATA_SLAVE;
    _ata_init_drive(&drives[1]);

    drives[2].io_bus = ATA_DEV_IO_SECONDARY;
    drives[2].ctr_bus = ATA_DEV_CTR_SECONDARY;
    drives[2].exist = 0;
    drives[2].master = ATA_MASTER;
    _ata_init_drive(&drives[2]);

    drives[3].io_bus = ATA_DEV_IO_SECONDARY;
    drives[3].ctr_bus = ATA_DEV_CTR_SECONDARY;
    drives[3].exist = 0;
    drives[3].master = ATA_SLAVE;
    _ata_init_drive(&drives[3]);

    /*
        TEST PURPOSE ONLY check if we can read the first sector of the boot drive.
    */
    if(_ata_read_sector(&buffer, 0, &drives[0]) == 0)
    {
        KERNEL_LOG_INFO("read success 0%p | magic number : 0%x", &buffer, buffer[255]);
    }

}