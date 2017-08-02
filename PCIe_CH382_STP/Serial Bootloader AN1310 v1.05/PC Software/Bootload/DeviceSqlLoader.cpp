/************************************************************************
* Copyright (c) 2010-2011,  Microchip Technology Inc.
*
* Microchip licenses this software to you solely for use with Microchip
* products.  The software is owned by Microchip and its licensors, and
* is protected under applicable copyright laws.  All rights reserved.
*
* SOFTWARE IS PROVIDED "AS IS."  MICROCHIP EXPRESSLY DISCLAIMS ANY
* WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL
* MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR
* EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY
* OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED
* TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
* OR OTHER SIMILAR COSTS.
*
* To the fullest extent allowed by law, Microchip and its licensors
* liability shall not exceed the amount of fees, if any, that you
* have paid directly to Microchip to use this software.
*
* MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
* OF THESE TERMS.
*
* Author        Date        Comment
*************************************************************************
* E. Schlunder  2010/02/13  Moved SQL code out of Device class so that
*                           XML converter tool can avoid linking to
*                           Qt SQL libraries.
************************************************************************/

#undef WITHQTSQL

#ifdef WITHQTSQL
#include <QtSql>

#include "DeviceSqlLoader.h"

#define DBFILE "devices.db"

DeviceSqlLoader::DeviceSqlLoader()
{
}

DeviceSqlLoader::ErrorCode DeviceSqlLoader::loadDevice(Device* device, QString partName)
{
    QString msg;
    bool deviceFound = false;

    device->setUnknown();

    // Try to find the "devices.db" database in the current working directory.
    QString deviceDatabaseFile = DBFILE;
    QFileInfo fi(deviceDatabaseFile);
    if(fi.exists() == false)
    {
        // Didn't find it, try looking in the application's EXE directory.
        deviceDatabaseFile = QCoreApplication::applicationDirPath() + QDir::separator() + DBFILE;
        if(QFileInfo(deviceDatabaseFile).exists() == false)
        {
            return DatabaseMissing;
        }
    }

    // the following opening code block provides a limited stack context for the
    // database connection, allowing us to disconnect from the database when
    // we are done.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "devices");
        db.setDatabaseName(deviceDatabaseFile);
        db.open();
        db.transaction();
        QSqlQuery qry(db);
        qry.setForwardOnly(true);
        msg = "select DEVICEID, FAMILYID\n" \
                    "from DEVICES\n" \
                    "where PARTNAME = :partName";
        qry.prepare(msg);
        msg.clear();

        qry.bindValue(0, partName);
        qry.exec();
        deviceFound = qry.next();
        if(deviceFound)
        {
            device->id = Device::toInt(qry.value(0));
            device->family = (Device::Families)Device::toInt(qry.value(1));
        }
        db.close();
    }
    QSqlDatabase::removeDatabase("devices");

    if(deviceFound)
    {
        return loadDevice(device, device->id, device->family);
    }

    return DeviceMissing;
}

QStringList DeviceSqlLoader::findDevices(QString query)
{
    QStringList results;
    QString msg;

    // Try to find the "devices.db" database in the current working directory.
    QString deviceDatabaseFile = DBFILE;
    QFileInfo fi(deviceDatabaseFile);
    if(fi.exists() == false)
    {
        // Didn't find it, try looking in the application's EXE directory.
        deviceDatabaseFile = QCoreApplication::applicationDirPath() + QDir::separator() + DBFILE;
        if(QFileInfo(deviceDatabaseFile).exists() == false)
        {
            return results;
        }
    }

    // the following opening code block provides a limited stack context for the
    // database connection, allowing us to disconnect from the database when
    // we are done.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "devices");
        db.setDatabaseName(deviceDatabaseFile);
        db.open();
        db.transaction();
        QSqlQuery qry(db);
        qry.setForwardOnly(true);
        msg = "select PARTNAME from DEVICES where PARTNAME like :query order by PARTNAME";
        qry.prepare(msg);
        msg.clear();

        qry.bindValue(0, "%" + query + "%");
        qry.exec();
        while(qry.next())
        {
            results.append(qry.value(0).toString());
        }
        qry.clear();

        db.commit();
        db.close();
    }
    QSqlDatabase::removeDatabase("devices");

    return results;
}

DeviceSqlLoader::ErrorCode DeviceSqlLoader::loadDevice(Device* device, int deviceId, Device::Families familyId)
{
    unsigned int deviceRowId;
    int i, j, t;
    QString msg;
    bool deviceFound = false;

    device->setUnknown();
    device->id = deviceId;

    // Try to find the "devices.db" database in the current working directory.
    QString deviceDatabaseFile = DBFILE;
    QFileInfo fi(deviceDatabaseFile);
    if(fi.exists() == false)
    {
        // Didn't find it, try looking in the application's EXE directory.
        deviceDatabaseFile = QCoreApplication::applicationDirPath() + QDir::separator() + DBFILE;
        if(QFileInfo(deviceDatabaseFile).exists() == false)
        {
            return DatabaseMissing;
        }
    }

    // the following opening code block provides a limited stack context for the
    // database connection, allowing us to disconnect from the database when
    // we are done.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "devices");
        db.setDatabaseName(deviceDatabaseFile);
        db.open();
        db.transaction();
        QSqlQuery qry(db);
        qry.setForwardOnly(true);
        msg = "select PARTNAME, WRITEFLASHBLOCKSIZE, ERASEFLASHBLOCKSIZE, STARTFLASH, ENDFLASH," \
                    "STARTEE, ENDEE, STARTUSER, ENDUSER, STARTCONFIG, ENDCONFIG, STARTGPR, ENDGPR," \
                    "BYTESPERWORDFLASH, FAMILYID, DEVICEROWID\n" \
                    "from DEVICES\n" \
                    "where ";
        if(familyId != Device::Unknown)
        {
            msg.append(" FAMILYID = ");
            msg.append(QString::number((int)familyId));
            msg.append(" and ");
        }
        msg.append("DEVICEID = :deviceId");
        qry.prepare(msg);
        msg.clear();

        qry.bindValue(0, deviceId);
        qry.exec();
        deviceFound = qry.next();
        if(deviceFound)
        {
            device->name = qry.value(0).toString();
            device->writeBlockSizeFLASH = Device::toInt(qry.value(1));
            device->eraseBlockSizeFLASH = Device::toInt(qry.value(2));
            device->startFLASH  = Device::toInt(qry.value(3));
            device->endFLASH    = Device::toInt(qry.value(4));
            device->startEEPROM = Device::toInt(qry.value(5));
            device->endEEPROM   = Device::toInt(qry.value(6));
            device->startUser   = Device::toInt(qry.value(7));
            device->endUser     = Device::toInt(qry.value(8));
            device->startConfig = Device::toInt(qry.value(9));
            device->endConfig   = Device::toInt(qry.value(10));
            device->startGPR    = Device::toInt(qry.value(11));
            device->endGPR      = Device::toInt(qry.value(12));
            device->bytesPerWordFLASH = Device::toInt(qry.value(13));
            device->family = (Device::Families)Device::toInt(qry.value(14));
            switch(device->family)
            {
                case Device::PIC16:
                    device->bytesPerAddressFLASH = 2;
                    device->bytesPerWordEEPROM = 1;
                    device->flashWordMask = 0x3FFF;
                    device->configWordMask = 0xFF;

                    break;

                case Device::PIC24:
                    device->bytesPerAddressFLASH = 2;
                    device->bytesPerWordEEPROM = 2;
                    device->flashWordMask = 0xFFFFFF;
                    device->configWordMask = 0xFFFF;
                    device->writeBlockSizeFLASH *= 2;       // temporary
                    device->eraseBlockSizeFLASH *= 2;
                    break;

                case Device::PIC32:
                    device->flashWordMask = 0xFFFFFFFF;
                    device->configWordMask = 0xFFFFFFFF;
                    device->bytesPerAddressFLASH = 1;
                    break;

                case Device::PIC18:
                default:
                    device->flashWordMask = 0xFFFF;
                    device->configWordMask = 0xFF;
                    device->bytesPerAddressFLASH = 1;
                    device->bytesPerWordEEPROM = 1;
            }
            deviceRowId = Device::toUInt(qry.value(15));
            qry.clear();

            qry.prepare("select ROWID, CONFIGNAME, ADDRESS, DEFAULTVALUE, IMPLEMENTEDBITS\n" \
                        "from CONFIGWORDS\n" \
                        "where DEVICEROWID = :deviceRowId\n" \
                        "order by ADDRESS");
            qry.bindValue(0, deviceRowId);
            qry.exec();
            Device::ConfigWord config;
            while(qry.next())
            {
                config.rowId = Device::toUInt(qry.value(0));
                config.name = qry.value(1).toString();
                config.address = Device::toUInt(qry.value(2));
                config.defaultValue = Device::toUInt(qry.value(3));
                config.implementedBits = Device::toUInt(qry.value(4));

                device->configWords.append(config);
            }
            qry.clear();

            qry.prepare("select ROWID, FIELDCNAME, DESCRIPTION, CONFIGWORDID\n" \
                        "from CONFIGFIELDS\n" \
                        "where DEVICEROWID = :deviceRowId\n" \
                        "order by CONFIGWORDID");
            qry.bindValue(0, deviceRowId);
            qry.exec();
            Device::ConfigField field;
            Device::ConfigWord* configWord = NULL;
            unsigned int id;
            while(qry.next())
            {
                field.rowId = Device::toUInt(qry.value(0));
                field.cname = qry.value(1).toString();
                field.description = qry.value(2).toString();
                id = Device::toUInt(qry.value(3).toString());

                if(configWord == NULL || configWord->rowId != id)
                {
                    for(i = 0; i < device->configWords.count(); i++)
                    {
                        if(device->configWords[i].rowId == id)
                        {
                            configWord = &device->configWords[i];
                            break;
                        }
                    }
                }
                configWord->fields.append(field);
            }
            qry.clear();

            qry.prepare("select SETTINGCNAME, DESCRIPTION, BITMASK, BITVALUE, CONFIGFIELDID\n" \
                        "from CONFIGSETTINGS\n" \
                        "where DEVICEROWID = :deviceRowId\n" \
                        "order by CONFIGFIELDID");
            qry.bindValue(0, deviceRowId);
            qry.exec();
            Device::ConfigFieldSetting setting;
            Device::ConfigField* parent = NULL;
            i = 0;
            bool searching;
            while(qry.next())
            {
                setting.cname = qry.value(0).toString();
                setting.description = qry.value(1).toString();
                setting.bitMask = Device::toUInt(qry.value(2));
                setting.bitValue = Device::toUInt(qry.value(3));
                id = Device::toUInt(qry.value(4));

                if(parent == NULL || parent->rowId != id)
                {
                    searching = true;
                    for(t = 0; searching && t < device->configWords.count(); t++, i++)
                    {
                        if(i >= device->configWords.count())
                        {
                            i = 0;
                        }
                        configWord = &device->configWords[i];

                        for(j = 0; searching && j < configWord->fields.count(); j++)
                        {
                            if(configWord->fields[j].rowId == id)
                            {
                                parent = &configWord->fields[j];
                                searching = false;
                            }
                        }
                    }
                }
                parent->settings.append(setting);
            }
        }
        qry.clear();

        db.commit();
        db.close();
    }
    QSqlDatabase::removeDatabase("devices");

    if(!deviceFound)
    {
        return DeviceMissing;
    }

    return Success;
}
#else /* WITHQTSQL */

#include "DeviceSqlLoader.h"

DeviceSqlLoader::DeviceSqlLoader()
{
}

DeviceSqlLoader::ErrorCode DeviceSqlLoader::loadDevice(Device* device, QString partName)
{
	return DeviceMissing;
}

DeviceSqlLoader::ErrorCode DeviceSqlLoader::loadDevice(Device* device, int deviceId, Device::Families familyId)
{
	if ((deviceId == 0x00000109) && (familyId == Device::PIC18)){
		/* --- generated by device.dump() --- */
		device->bytesPerWordEEPROM = 1;
		device->blankValue = 0xffffffff;
		device->flashWordMask = 0x0000ffff;
		device->configWordMask = 0x000000ff;
		device->name = "PIC18F2321";
		device->bytesPerAddressFLASH = 1;
		device->writeBlockSizeFLASH = 8;
		device->eraseBlockSizeFLASH = 64;
		device->startFLASH  = 0x00000000;
		device->endFLASH    = 0x00002000;
		device->startBootloader = 0x00000000;
		device->endBootloader = 0x00000000;
		device->startEEPROM = 0x00f00000;
		device->endEEPROM   = 0x00f00100;
		device->startUser   = 0x00200000;
		device->endUser     = 0x00200008;
		device->startConfig = 0x00300000;
		device->endConfig   = 0x0030000e;
		device->startGPR    = 0x00000000;
		device->endGPR      = 0x00000200;
		device->startIVT    = 0x00000004;
		device->endIVT      = 0x00000100;
		device->startAIVT   = 0x00000104;
		device->endAIVT     = 0x00000200;
		device->bytesPerWordFLASH = 2;
		
		Device::ConfigWord word;
		Device::ConfigField field;
		Device::ConfigFieldSetting setting;
		
		word.rowId = 0x0000054d;
		word.name = "CONFIG1H";
		word.address = 0x00300001;
		word.defaultValue = 0x00000007;
		word.implementedBits= 0x000000cf;
		device->configWords.append(word);
		field.rowId = 0x0000126d;
		field.name = "";
		field.cname = "OSC";
		field.description = "Oscillator";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[0].fields.append(field);
		setting.name = "";
		setting.cname = "";
		setting.description = "11XX External RC oscillator, CLKO function on RA6";
		setting.bitMask = 0x0000000c;
		setting.bitValue = 0x0000000c;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "";
		setting.description = "101X External RC oscillator, CLKO function on RA6";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x0000000a;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "INTIO1";
		setting.description = "Internal oscillator block, CLKO function on RA6, port function on RA7";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000009;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "INTIO2";
		setting.description = "Internal oscillator block, port function on RA6 and RA7";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000008;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "RCIO";
		setting.description = "External RC oscillator, port function on RA6";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000007;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "HSPLL";
		setting.description = "HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1)";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000006;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ECIO";
		setting.description = "EC oscillator, port function on RA6";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000005;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "EC";
		setting.description = "EC oscillator, CLKO function on RA6";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000004;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "RC";
		setting.description = "External RC oscillator, CLKO function on RA6";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000003;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "HS";
		setting.description = "HS Oscillator";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000002;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "XT";
		setting.description = "XT Oscillator";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000001;
		device->configWords[0].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "LP";
		setting.description = "LP Oscillator";
		setting.bitMask = 0x0000000f;
		setting.bitValue = 0x00000000;
		device->configWords[0].fields[0].settings.append(setting);
		field.rowId = 0x0000126e;
		field.name = "";
		field.cname = "FCMEN";
		field.description = "Fail-Safe Clock Monitor Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[0].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Fail-Safe Clock Monitor enabled";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000040;
		device->configWords[0].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Fail-Safe Clock Monitor disabled";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000000;
		device->configWords[0].fields[1].settings.append(setting);
		field.rowId = 0x0000126f;
		field.name = "";
		field.cname = "IESO";
		field.description = "Internal/External Oscillator Switchover bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[0].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Oscillator Switchover mode enabled";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000080;
		device->configWords[0].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Oscillator Switchover mode disabled";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000000;
		device->configWords[0].fields[2].settings.append(setting);
		word.rowId = 0x0000054e;
		word.name = "CONFIG2L";
		word.address = 0x00300002;
		word.defaultValue = 0x0000001f;
		word.implementedBits= 0x0000001f;
		device->configWords.append(word);
		field.rowId = 0x00001270;
		field.name = "";
		field.cname = "PWRT";
		field.description = "Power-up Timer Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[1].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "PWRT disabled";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[1].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "PWRT enabled";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[1].fields[0].settings.append(setting);
		field.rowId = 0x00001271;
		field.name = "";
		field.cname = "BOR";
		field.description = "Brown-out Reset Enable bits";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[1].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Brown-out Reset enabled in hardware only (SBOREN is disabled)";
		setting.bitMask = 0x00000006;
		setting.bitValue = 0x00000006;
		device->configWords[1].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "NOSLP";
		setting.description = "Brown-out Reset enabled in hardware only and disabled in Sleep mode (SBOREN is disabled)";
		setting.bitMask = 0x00000006;
		setting.bitValue = 0x00000004;
		device->configWords[1].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "SOFT";
		setting.description = "Brown-out Reset enabled and controlled by software (SBOREN is enabled)";
		setting.bitMask = 0x00000006;
		setting.bitValue = 0x00000002;
		device->configWords[1].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Brown-out Reset disabled in hardware and software";
		setting.bitMask = 0x00000006;
		setting.bitValue = 0x00000000;
		device->configWords[1].fields[1].settings.append(setting);
		field.rowId = 0x00001272;
		field.name = "";
		field.cname = "BORV";
		field.description = "Brown-out Reset Voltage bits";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[1].fields.append(field);
		setting.name = "";
		setting.cname = "3";
		setting.description = "Minimum Setting";
		setting.bitMask = 0x00000018;
		setting.bitValue = 0x00000018;
		device->configWords[1].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "2";
		setting.description = "";
		setting.bitMask = 0x00000018;
		setting.bitValue = 0x00000010;
		device->configWords[1].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "1";
		setting.description = "";
		setting.bitMask = 0x00000018;
		setting.bitValue = 0x00000008;
		device->configWords[1].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "0";
		setting.description = "Maximum Setting";
		setting.bitMask = 0x00000018;
		setting.bitValue = 0x00000000;
		device->configWords[1].fields[2].settings.append(setting);
		word.rowId = 0x0000054f;
		word.name = "CONFIG2H";
		word.address = 0x00300003;
		word.defaultValue = 0x0000001f;
		word.implementedBits= 0x0000001f;
		device->configWords.append(word);
		field.rowId = 0x00001273;
		field.name = "";
		field.cname = "WDT";
		field.description = "Watchdog Timer Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[2].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "WDT enabled";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[2].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "WDT disabled (control is placed on the SWDTEN bit)";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[2].fields[0].settings.append(setting);
		field.rowId = 0x00001274;
		field.name = "";
		field.cname = "WDTPS";
		field.description = "Watchdog Timer Postscale Select bits";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[2].fields.append(field);
		setting.name = "";
		setting.cname = "32768";
		setting.description = "1:32768";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000001e;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "16384";
		setting.description = "1:16384";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000001c;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "8192";
		setting.description = "1:8192";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000001a;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "4096";
		setting.description = "1:4096";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000018;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "2048";
		setting.description = "1:2048";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000016;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "1024";
		setting.description = "1:1024";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000014;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "512";
		setting.description = "1:512";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000012;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "256";
		setting.description = "1:256";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000010;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "128";
		setting.description = "1:128";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000000e;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "64";
		setting.description = "1:64";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000000c;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "32";
		setting.description = "1:32";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x0000000a;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "16";
		setting.description = "1:16";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000008;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "8";
		setting.description = "1:8";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000006;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "4";
		setting.description = "1:4";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000004;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "2";
		setting.description = "1:2";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000002;
		device->configWords[2].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "1";
		setting.description = "1:1";
		setting.bitMask = 0x0000001e;
		setting.bitValue = 0x00000000;
		device->configWords[2].fields[1].settings.append(setting);
		word.rowId = 0x00000550;
		word.name = "CONFIG3H";
		word.address = 0x00300005;
		word.defaultValue = 0x00000083;
		word.implementedBits= 0x00000087;
		device->configWords.append(word);
		field.rowId = 0x00001275;
		field.name = "";
		field.cname = "CCP2MX";
		field.description = "CCP2 MUX bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[3].fields.append(field);
		setting.name = "";
		setting.cname = "RC1";
		setting.description = "CCP2 input/output is multiplexed with RC1";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[3].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "RB3";
		setting.description = "CCP2 input/output is multiplexed with RB3";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[3].fields[0].settings.append(setting);
		field.rowId = 0x00001276;
		field.name = "";
		field.cname = "PBADEN";
		field.description = "PORTB A/D Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[3].fields.append(field);
		setting.name = "";
		setting.cname = "ANA";
		setting.description = "PORTB<4:0> pins are configured as analog input channels on Reset";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000002;
		device->configWords[3].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "DIG";
		setting.description = "PORTB<4:0> pins are configured as digital I/O on Reset";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000000;
		device->configWords[3].fields[1].settings.append(setting);
		field.rowId = 0x00001277;
		field.name = "";
		field.cname = "LPT1OSC";
		field.description = "Low-Power Timer1 Oscillator Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[3].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Timer1 configured for low-power operation";
		setting.bitMask = 0x00000004;
		setting.bitValue = 0x00000004;
		device->configWords[3].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Timer1 configured for higher power operation";
		setting.bitMask = 0x00000004;
		setting.bitValue = 0x00000000;
		device->configWords[3].fields[2].settings.append(setting);
		field.rowId = 0x00001278;
		field.name = "";
		field.cname = "MCLRE";
		field.description = "MCLR Pin Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[3].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "MCLR pin enabled; RE3 input pin disabled";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000080;
		device->configWords[3].fields[3].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "RE3 input pin enabled; MCLR disabled";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000000;
		device->configWords[3].fields[3].settings.append(setting);
		word.rowId = 0x00000551;
		word.name = "CONFIG4L";
		word.address = 0x00300006;
		word.defaultValue = 0x00000085;
		word.implementedBits= 0x000000fd;
		device->configWords.append(word);
		field.rowId = 0x00001279;
		field.name = "";
		field.cname = "STVREN";
		field.description = "Stack Full/Underflow Reset Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[4].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Stack full/underflow will cause Reset";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[4].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Stack full/underflow will not cause Reset";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[4].fields[0].settings.append(setting);
		field.rowId = 0x0000127a;
		field.name = "";
		field.cname = "LVP";
		field.description = "Single-Supply ICSP Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[4].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Single-Supply ICSP enabled";
		setting.bitMask = 0x00000004;
		setting.bitValue = 0x00000004;
		device->configWords[4].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Single-Supply ICSP disabled";
		setting.bitMask = 0x00000004;
		setting.bitValue = 0x00000000;
		device->configWords[4].fields[1].settings.append(setting);
		field.rowId = 0x0000127b;
		field.name = "";
		field.cname = "BBSIZ";
		field.description = "Boot Block Size Select bits";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[4].fields.append(field);
		setting.name = "";
		setting.cname = "BB1K";
		setting.description = "1024 Word";
		setting.bitMask = 0x00000030;
		setting.bitValue = 0x00000030;
		device->configWords[4].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "";
		setting.description = "1024 Word";
		setting.bitMask = 0x00000030;
		setting.bitValue = 0x00000020;
		device->configWords[4].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "BB512";
		setting.description = " 512 Word";
		setting.bitMask = 0x00000030;
		setting.bitValue = 0x00000010;
		device->configWords[4].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "BB256";
		setting.description = " 256 Word";
		setting.bitMask = 0x00000030;
		setting.bitValue = 0x00000000;
		device->configWords[4].fields[2].settings.append(setting);
		field.rowId = 0x0000127c;
		field.name = "";
		field.cname = "XINST";
		field.description = "Extended Instruction Set Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[4].fields.append(field);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Instruction set extension and Indexed Addressing mode enabled";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000040;
		device->configWords[4].fields[3].settings.append(setting);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Instruction set extension and Indexed Addressing mode disabled (Legacy mode)";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000000;
		device->configWords[4].fields[3].settings.append(setting);
		field.rowId = 0x0000127d;
		field.name = "";
		field.cname = "DEBUG";
		field.description = "Background Debugger Enable bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[4].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000080;
		device->configWords[4].fields[4].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Background debugger enabled, RB6 and RB7 are dedicated to In-Circuit Debug";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000000;
		device->configWords[4].fields[4].settings.append(setting);
		word.rowId = 0x00000552;
		word.name = "CONFIG5L";
		word.address = 0x00300008;
		word.defaultValue = 0x00000003;
		word.implementedBits= 0x00000003;
		device->configWords.append(word);
		field.rowId = 0x0000127e;
		field.name = "";
		field.cname = "CP0";
		field.description = "Code Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[5].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 0 not code-protected";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[5].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 0 code-protected";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[5].fields[0].settings.append(setting);
		field.rowId = 0x0000127f;
		field.name = "";
		field.cname = "CP1";
		field.description = "Code Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[5].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 1 not code-protected";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000002;
		device->configWords[5].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 1 code-protected";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000000;
		device->configWords[5].fields[1].settings.append(setting);
		word.rowId = 0x00000553;
		word.name = "CONFIG5H";
		word.address = 0x00300009;
		word.defaultValue = 0x000000c0;
		word.implementedBits= 0x000000c0;
		device->configWords.append(word);
		field.rowId = 0x00001280;
		field.name = "";
		field.cname = "CPB";
		field.description = "Boot Block Code Protection bitProtect Boot";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[6].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Boot block not code-protected";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000040;
		device->configWords[6].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Boot block code-protected";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000000;
		device->configWords[6].fields[0].settings.append(setting);
		field.rowId = 0x00001281;
		field.name = "";
		field.cname = "CPD";
		field.description = "Data EEPROM Code Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[6].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Data EEPROM not code-protected";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000080;
		device->configWords[6].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Data EEPROM code-protected";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000000;
		device->configWords[6].fields[1].settings.append(setting);
		word.rowId = 0x00000554;
		word.name = "CONFIG6L";
		word.address = 0x0030000a;
		word.defaultValue = 0x00000003;
		word.implementedBits= 0x00000003;
		device->configWords.append(word);
		field.rowId = 0x00001282;
		field.name = "";
		field.cname = "WRT0";
		field.description = "Write Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[7].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 0 not write-protected";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[7].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 0 write-protected";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[7].fields[0].settings.append(setting);
		field.rowId = 0x00001283;
		field.name = "";
		field.cname = "WRT1";
		field.description = "Write Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[7].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 1 not write-protected";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000002;
		device->configWords[7].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 1 write-protected";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000000;
		device->configWords[7].fields[1].settings.append(setting);
		word.rowId = 0x00000555;
		word.name = "CONFIG6H";
		word.address = 0x0030000b;
		word.defaultValue = 0x000000e0;
		word.implementedBits= 0x000000e0;
		device->configWords.append(word);
		field.rowId = 0x00001284;
		field.name = "";
		field.cname = "WRTC";
		field.description = "Configuration Register Write Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[8].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Configuration registers (300000-3000FFh) not write-protected";
		setting.bitMask = 0x00000020;
		setting.bitValue = 0x00000020;
		device->configWords[8].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Configuration registers (300000-3000FFh) write-protected";
		setting.bitMask = 0x00000020;
		setting.bitValue = 0x00000000;
		device->configWords[8].fields[0].settings.append(setting);
		field.rowId = 0x00001285;
		field.name = "";
		field.cname = "WRTB";
		field.description = "Boot Block Write Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[8].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Boot block not write-protected";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000040;
		device->configWords[8].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Boot block write-protected";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000000;
		device->configWords[8].fields[1].settings.append(setting);
		field.rowId = 0x00001286;
		field.name = "";
		field.cname = "WRTD";
		field.description = "Data EEPROM Write Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[8].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Data EEPROM not write-protected";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000080;
		device->configWords[8].fields[2].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Data EEPROM write-protected";
		setting.bitMask = 0x00000080;
		setting.bitValue = 0x00000000;
		device->configWords[8].fields[2].settings.append(setting);
		word.rowId = 0x00000556;
		word.name = "CONFIG7L";
		word.address = 0x0030000c;
		word.defaultValue = 0x00000003;
		word.implementedBits= 0x00000003;
		device->configWords.append(word);
		field.rowId = 0x00001287;
		field.name = "";
		field.cname = "EBTR0";
		field.description = "Table Read Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[9].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 0 not protected from table reads executed in other blocks";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000001;
		device->configWords[9].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 0 protected from table reads executed in other blocks";
		setting.bitMask = 0x00000001;
		setting.bitValue = 0x00000000;
		device->configWords[9].fields[0].settings.append(setting);
		field.rowId = 0x00001288;
		field.name = "";
		field.cname = "EBTR1";
		field.description = "Table Read Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[9].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Block 1 not protected from table reads executed in other blocks";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000002;
		device->configWords[9].fields[1].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Block 1 protected from table reads executed in other blocks";
		setting.bitMask = 0x00000002;
		setting.bitValue = 0x00000000;
		device->configWords[9].fields[1].settings.append(setting);
		word.rowId = 0x00000557;
		word.name = "CONFIG7H";
		word.address = 0x0030000d;
		word.defaultValue = 0x00000040;
		word.implementedBits= 0x00000040;
		device->configWords.append(word);
		field.rowId = 0x00001289;
		field.name = "";
		field.cname = "EBTRB";
		field.description = "Boot Block Table Read Protection bit";
		field.mask = 0x0936e7d8;
		field.width = 0x00000002;
		field.hidden = 0x00000004;
		device->configWords[10].fields.append(field);
		setting.name = "";
		setting.cname = "OFF";
		setting.description = "Boot block not protected from table reads executed in other blocks";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000040;
		device->configWords[10].fields[0].settings.append(setting);
		setting.name = "";
		setting.cname = "ON";
		setting.description = "Boot block protected from table reads executed in other blocks";
		setting.bitMask = 0x00000040;
		setting.bitValue = 0x00000000;
		device->configWords[10].fields[0].settings.append(setting);
		/* --- generated by device.dump() --- */
	
		return Success;
	}

	return DeviceMissing;

}
#endif
