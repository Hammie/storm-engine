#include "teleport.h"

#include "core.h"

#include "../Pcs_controls/pcs_controls.h"
#include "defines.h"
#include "vfile_service.h"

#define DELETE_PTR(x)                                                                                                  \
    if (x)                                                                                                             \
        delete x;                                                                                                      \
    x = 0;

bool GetStringLine(char *&pStr, char *bufer, long bufSize)
{
    if (pStr == nullptr || bufer == nullptr || bufSize == 0)
        return false;
    bufer[0] = 0;

    auto *ps = pStr;
    while (*ps && (*ps == 32 || *ps == 9 || *ps == 10 || *ps == 13))
        ps++;
    auto *const pStart = ps;

    while (*ps && *ps != 10 && *ps != 13)
        ps++;
    auto *pEnd = ps;
    pStr = pEnd;

    if (pEnd == pStart && *ps == 0)
        return false;
    if (static_cast<long>(pEnd - pStart) > bufSize - 1)
        pEnd = pStart + bufSize - 1;
    else
        bufSize = pEnd - pStart;

    if (bufSize > 0)
    {
        strcpy_s(bufer, bufSize, pStart);
        bufer[bufSize] = 0;
    }
    return true;
}

void GetQuotedString(char *inBuf, char *outBuf, long bufSize)
{
    if (outBuf == nullptr || bufSize <= 0)
        return;
    *outBuf = 0;
    if (inBuf == nullptr)
        return;

    while (*inBuf && *inBuf != '\"')
        inBuf++;
    if (*inBuf)
        inBuf++;

    auto bufIdx = 0;
    while (*inBuf && *inBuf != '\"' && bufIdx < bufSize - 1)
    {
        *outBuf = *inBuf;
        outBuf++;
        inBuf++;
        bufIdx++;
    }
    *outBuf = 0;
}

TMPTELEPORT::TMPTELEPORT()
{
    rs = nullptr;
    m_nStrQuantity = m_nCurStr = m_nCurShowPos = 0;
}

TMPTELEPORT::~TMPTELEPORT()
{
    ReleaseAll();
}

bool TMPTELEPORT::Init()
{
    rs = static_cast<VDX9RENDER *>(core.CreateService("dx9render"));
    if (!rs)
        throw std::exception("No service: dx9render");

    m_leftPos = 20;
    m_topPos = 80;
    m_deltaPos = 30;
    m_showStrQuantity = 10;

    m_nShowType = 0;

    return true;
}

void TMPTELEPORT::Execute(uint32_t Delta_Time)
{
    CONTROL_STATE cs;
    if (static_cast<PCS_CONTROLS *>(core.Controls)->m_bIsOffDebugKeys)
        return;
    core.Controls->GetControlState("TeleportActive", cs);
    if (cs.state == CST_ACTIVATED)
    {
        if (m_nShowType == 0)
        {
            core.Event("TeleportStart", "");
            m_nShowType = 1;
        }
        else
        {
            ReleaseAll();
            m_nShowType = 0;
        }
    }
    long csVal;
    if (core.Controls->GetDebugAsyncKeyState(VK_SHIFT) < 0)
        csVal = CST_ACTIVE;
    else
        csVal = CST_ACTIVATED;
    core.Controls->GetControlState("TeleportUp", cs);
    if (cs.state == csVal)
    {
        if (m_nStrQuantity > 0)
        {
            if (m_nCurShowPos > 0)
                m_nCurShowPos--;
            else if (m_nCurStr > 0)
                m_nCurStr--;
        }
    }
    core.Controls->GetControlState("TeleportDown", cs);
    if (cs.state == csVal)
    {
        if (m_nStrQuantity > 0)
        {
            if (m_nCurStr + m_nCurShowPos < m_nStrQuantity - 1)
                if (m_nCurShowPos < m_showStrQuantity - 1)
                    m_nCurShowPos++;
                else
                    m_nCurStr++;
        }
    }
    core.Controls->GetControlState("TeleportSelect", cs);
    if (cs.state == CST_ACTIVATED)
    {
        if (m_nStrQuantity > 0)
        {
            const long n = m_descrArray[m_nCurStr + m_nCurShowPos].num;
            ReleaseAll();
            core.Event("TeleportChoose", "l", n);
        }
    }
}

void TMPTELEPORT::Realize(uint32_t Delta_Time)
{
    if (m_nStrQuantity > 0)
    {
        auto j = 0;
        auto ftop = m_topPos;
        for (int i = m_nCurStr; i < m_nStrQuantity; i++)
        {
            if (j >= m_showStrQuantity)
                break;
            if (j == m_nCurShowPos)
                rs->Print(FONT_DEFAULT, ARGB(255, 155, 155, 55), m_leftPos, ftop, "%s", m_descrArray[i].name);
            else
                rs->Print(FONT_DEFAULT, ARGB(255, 255, 255, 255), m_leftPos, ftop, "%s", m_descrArray[i].name);
            ftop += m_deltaPos;
            j++;
        }
    }
}

void TMPTELEPORT::ReleaseAll()
{
    m_descrArray.clear();
    m_nStrQuantity = 0;
    m_nCurStr = m_nCurShowPos = 0;
    m_nShowType = 0;
}

uint64_t TMPTELEPORT::ProcessMessage(MESSAGE &message)
{
    switch (message.Long())
    {
    case 42222: {
        auto *const pA = message.AttributePointer();
        SetShowData(pA);
        if (m_nStrQuantity == 0)
            m_nShowType = 0;
        else
            m_nShowType = 1;
    }
    break;
    }
    return 0;
}

void TMPTELEPORT::SetShowData(Attribute *pA)
{
    ReleaseAll();
    m_nStrQuantity = 0;
    if (pA == nullptr)
        return;

    Assert(pA != nullptr);
    const Attribute& attr = *pA;

    if (attr.empty()) {
        return;
    }

    int i = 0;
    std::transform(attr.begin(), attr.end(), std::back_inserter(m_descrArray), [&i] (const Attribute& entry) {
      TELEPORT_DESCR result;
      result.num = i++;
      entry.get_to(result.name);
      return result;
    });

    SortShowData();
}

void TMPTELEPORT::SortShowData()
{
    if (m_nStrQuantity == 0)
        return;
    auto bContinueSort = true;
    do
    {
        bContinueSort = false;
        for (auto i = 1; i < m_nStrQuantity; i++)
        {
            if (m_descrArray[i].name == m_descrArray[i - 1].name)
            {
                XChange(m_descrArray[i - 1], m_descrArray[i]);
                bContinueSort = true;
            }
        }
    } while (bContinueSort);
}

void TMPTELEPORT::XChange(TELEPORT_DESCR &d1, TELEPORT_DESCR &d2)
{
    std::swap(d1.num, d2.num);
    std::swap(d1.name, d2.name);
}

bool FINDFILESINTODIRECTORY::Init()
{
    if (AttributesPointer)
    {
        Assert(AttributesPointer != nullptr);
        Attribute& attr = *AttributesPointer;
        std::string fullName = attr["dir"].get<std::string>() + '\\' + attr["mash"].get<std::string>("*.*");
        WIN32_FIND_DATA finddat;
        auto *const hdl = fio->_FindFirstFile(fullName.c_str(), &finddat);
        Attribute& aFileList = attr["filelist"];
        for (auto file_idx = 0; hdl != INVALID_HANDLE_VALUE; file_idx++)
        {
            char sname[32];
            sprintf_s(sname, "id%d", file_idx);
            if (finddat.cFileName)
            {
                std::string FileName = utf8::ConvertWideToUtf8(finddat.cFileName);
                aFileList[sname] = FileName.c_str();
            }
            if (!fio->_FindNextFile(hdl, &finddat))
                break;
        }
        if (hdl != INVALID_HANDLE_VALUE)
            fio->_FindClose(hdl);
        return true;
    }
    core.Trace("Attributes Pointer into class FINDFILESINTODIRECTORY = NULL");
    return false;
}

bool FINDDIALOGNODES::Init()
{
    if (AttributesPointer)
    {
        Assert(AttributesPointer != nullptr);
        Attribute& attr = *AttributesPointer;

        auto *const fileName = attr["file"].get<const char*>();
        Attribute &aNodeList = attr["nodelist"];
        if (fileName && !aNodeList.empty())
        {
            auto *const hfile = fio->_CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
            if (hfile == INVALID_HANDLE_VALUE)
            {
                core.Trace("WARNING! Can`t dialog file %s", fileName);
                return false;
            }

            const long filesize = fio->_GetFileSize(hfile, nullptr);
            if (filesize == 0)
            {
                core.Trace("Empty dialog file %s", fileName);
                fio->_CloseHandle(hfile);
                return false;
            }

            auto *const fileBuf = new char[filesize + 1];
            if (fileBuf == nullptr)
            {
                core.Trace("Can`t create buffer for read dialog file %s", fileName);
                fio->_CloseHandle(hfile);
                return false;
            }

            uint32_t readsize;
            if (fio->_ReadFile(hfile, fileBuf, filesize, &readsize) == FALSE ||
                readsize != static_cast<uint32_t>(filesize))
            {
                core.Trace("Can`t read dialog file: %s", fileName);
                fio->_CloseHandle(hfile);
                delete[] fileBuf;
                return false;
            }
            fio->_CloseHandle(hfile);
            fileBuf[filesize] = 0;

            // now there is a buffer - start analyzing it
            auto *pStr = fileBuf;
            char param[1024];

            auto nodIdx = 0;
            while (GetStringLine(pStr, param, sizeof(param) - 1))
            {
                if (strlen(param) < 5 || _strnicmp(param, "case", 4))
                    continue;
                char param2[512];
                GetQuotedString(param, param2, sizeof(param2) - 1);
                if (strlen(param2) > 0)
                {
                    sprintf_s(param, "id%d", nodIdx);
                    nodIdx++;
                    aNodeList[param] = param2;
                }
            }

            delete[] fileBuf;
            return true;
        }
    }
    core.Trace("Attributes Pointer into class FINDDIALOGNODES = NULL");
    return false;
}
