#undef UNICODE
#undef _UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>

using namespace std;

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

struct MayBay
{
    char soHieuMB[16];
    char loaiMB[41];
    int soCho;
    int soLuotBay;
};

struct Ve
{
    char CCCD[15];
};

struct NgayGio
{
    int gio;
    int phut;
    int ngay;
    int thang;
    int nam;
};

struct ChuyenBay
{
    char maCB[16];
    char maMB[16];
    char sanBayDen[50];
    NgayGio tg;
    int trangThai;
    Ve *dsVe;
    int soCho;
};

struct HanhKhach
{
    char CCCD[15];
    char ho[51];
    char ten[16];
    char gioiTinh[5];
};

struct NodeHK
{
    HanhKhach data;
    NodeHK *left;
    NodeHK *right;
};

struct NodeCB
{
    ChuyenBay data;
    NodeCB *next;
};

struct DSMayBay
{
    int n = 0;
    MayBay *nodes[300];
};

DSMayBay dsMB;
NodeCB* headCB = NULL;
NodeHK* rootHK = NULL;

int currentModule = 1;
HWND hMain;
HWND hListView;
HWND hBtnMB;
HWND hBtnCB;
HWND hBtnHK;
HWND hBtnAdd;
HWND hBtnDel;
HWND hBtnBook;
HFONT hFontNormal;
HFONT hFontBold;
HFONT hFontTitle;
HBRUSH hBrushBg;

#define COL_BG RGB(240, 242, 245)
#define COL_SIDEBAR RGB(255, 255, 255)

string MyIntToStr(int n)
{
    char b[20];
    sprintf(b, "%d", n);
    return string(b);
}

void ChuanHoaTen(char* hoIn, char* tenIn, char* hoOut, char* tenOut)
{
    string s = hoIn;
    string res = "";
    bool up = true;
    for (size_t i = 0; i < s.length(); i++)
    {
        if (isspace(s[i]))
        {
            if (!res.empty() && res.back() != ' ')
            {
                res += ' ';
                up = true;
            }
        }
        else
        {
            if (up)
            {
                res += toupper(s[i]);
                up = false;
            }
            else
            {
                res += tolower(s[i]);
            }
        }
    }
    strcpy(hoOut, res.c_str());
    s = tenIn;
    string resTen = "";
    for (size_t i = 0; i < s.length(); i++)
    {
        if (!isspace(s[i]))
        {
            resTen += toupper(s[i]);
        }
    }
    strcpy(tenOut, resTen.c_str());
}

bool CheckMaMB(const char* ma)
{
    if (strlen(ma) < 3)
    {
        return false;
    }
    if (strncmp(ma, "QS", 2) != 0 && strncmp(ma, "CK", 2) != 0 && strncmp(ma, "HK", 2) != 0)
    {
        return false;
    }
    for (size_t i = 2; i < strlen(ma); i++)
    {
        if (!isdigit(ma[i]))
        {
            return false;
        }
    }
    return true;
}

bool CheckCCCD(const char* c)
{
    if (strlen(c) < 12 || strlen(c) > 13)
    {
        return false;
    }
    for (size_t i = 0; i < strlen(c); i++)
    {
        if (!isdigit(c[i]))
        {
            return false;
        }
    }
    return true;
}

void FormatDate(NgayGio t, char* buf)
{
    sprintf(buf, "%02d:%02d %02d/%02d/%04d", t.gio, t.phut, t.ngay, t.thang, t.nam);
}

bool ThemMB(MayBay mb)
{
    if (dsMB.n >= 300)
    {
        return false;
    }
    for (int i = 0; i < dsMB.n; i++)
    {
        if (strcmp(dsMB.nodes[i]->soHieuMB, mb.soHieuMB) == 0)
        {
            return false;
        }
    }
    dsMB.nodes[dsMB.n] = new MayBay;
    *dsMB.nodes[dsMB.n] = mb;
    dsMB.n++;
    return true;
}

void XoaMB(int idx)
{
    if (idx < 0 || idx >= dsMB.n)
    {
        return;
    }
    delete dsMB.nodes[idx];
    for (int i = idx; i < dsMB.n - 1; i++)
    {
        dsMB.nodes[i] = dsMB.nodes[i + 1];
    }
    dsMB.n--;
}

void ThemHK(NodeHK* &r, HanhKhach hk)
{
    if (!r)
    {
        r = new NodeHK;
        r->data = hk;
        r->left = NULL;
        r->right = NULL;
    }
    else if (strcmp(hk.CCCD, r->data.CCCD) < 0)
    {
        ThemHK(r->left, hk);
    }
    else if (strcmp(hk.CCCD, r->data.CCCD) > 0)
    {
        ThemHK(r->right, hk);
    }
}

NodeHK* TimHK(NodeHK* r, char* cccd)
{
    if (!r)
    {
        return NULL;
    }
    int cmp = strcmp(cccd, r->data.CCCD);
    if (cmp == 0)
    {
        return r;
    }
    if (cmp < 0)
    {
        return TimHK(r->left, cccd);
    }
    return TimHK(r->right, cccd);
}

void ThemCB(ChuyenBay cb)
{
    NodeCB* p = new NodeCB;
    p->data = cb;
    p->data.dsVe = new Ve[cb.soCho];
    for (int i = 0; i < cb.soCho; i++)
    {
        strcpy(p->data.dsVe[i].CCCD, "");
    }
    p->next = headCB;
    headCB = p;
}

bool DatVe(char* maCB, char* cccd)
{
    NodeCB* p = headCB;
    while (p && strcmp(p->data.maCB, maCB) != 0)
    {
        p = p->next;
    }
    if (!p || p->data.trangThai != 1)
    {
        return false;
    }
    for (int i = 0; i < p->data.soCho; i++)
    {
        if (strcmp(p->data.dsVe[i].CCCD, cccd) == 0)
        {
            return false;
        }
    }
    for (int i = 0; i < p->data.soCho; i++)
    {
        if (strlen(p->data.dsVe[i].CCCD) == 0)
        {
            strcpy(p->data.dsVe[i].CCCD, cccd);
            for (int k = 0; k < dsMB.n; k++)
            {
                if (strcmp(dsMB.nodes[k]->soHieuMB, p->data.maMB) == 0)
                {
                    dsMB.nodes[k]->soLuotBay++;
                }
            }
            return true;
        }
    }
    return false;
}

void SaveData()
{
    ofstream f1("maybay.dat", ios::binary);
    if (f1)
    {
        f1.write((char*)&dsMB.n, 4);
        for (int i = 0; i < dsMB.n; i++)
        {
            f1.write((char*)dsMB.nodes[i], sizeof(MayBay));
        }
        f1.close();
    }
    ofstream f2("chuyenbay.dat", ios::binary);
    if (f2)
    {
        int c = 0;
        NodeCB* p = headCB;
        while (p)
        {
            c++;
            p = p->next;
        }
        f2.write((char*)&c, 4);
        p = headCB;
        while (p)
        {
            f2.write((char*)&p->data, sizeof(ChuyenBay) - sizeof(Ve*) - sizeof(int));
            f2.write((char*)&p->data.soCho, 4);
            f2.write((char*)p->data.dsVe, sizeof(Ve) * p->data.soCho);
            p = p->next;
        }
        f2.close();
    }
    ofstream f3("khach.dat", ios::binary);
    if (f3)
    {
        vector<HanhKhach> v;
        auto Trav = [&](auto&& s, NodeHK* r) -> void {
            if (!r) return;
            s(s, r->left);
            v.push_back(r->data);
            s(s, r->right);
        };
        Trav(Trav, rootHK);
        int sz = v.size();
        f3.write((char*)&sz, 4);
        for (auto &h : v)
        {
            f3.write((char*)&h, sizeof(HanhKhach));
        }
        f3.close();
    }
}

void LoadData()
{
    ifstream f1("maybay.dat", ios::binary);
    if (f1)
    {
        f1.read((char*)&dsMB.n, 4);
        for (int i = 0; i < dsMB.n; i++)
        {
            dsMB.nodes[i] = new MayBay;
            f1.read((char*)dsMB.nodes[i], sizeof(MayBay));
        }
        f1.close();
    }
    ifstream f2("chuyenbay.dat", ios::binary);
    if (f2)
    {
        int c = 0;
        f2.read((char*)&c, 4);
        for (int i = 0; i < c; i++)
        {
            NodeCB* p = new NodeCB;
            f2.read((char*)&p->data, sizeof(ChuyenBay) - sizeof(Ve*) - sizeof(int));
            f2.read((char*)&p->data.soCho, 4);
            p->data.dsVe = new Ve[p->data.soCho];
            f2.read((char*)p->data.dsVe, sizeof(Ve) * p->data.soCho);
            p->next = headCB;
            headCB = p;
        }
        f2.close();
    }
    ifstream f3("khach.dat", ios::binary);
    if (f3)
    {
        int sz = 0;
        f3.read((char*)&sz, 4);
        for (int i = 0; i < sz; i++)
        {
            HanhKhach h;
            f3.read((char*)&h, sizeof(HanhKhach));
            ThemHK(rootHK, h);
        }
        f3.close();
    }
}

HFONT CreateFontUI(int size, bool bold)
{
    return CreateFont(size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
}

void ResizeLayout(int w, int h)
{
    int sideW = 200;
    MoveWindow(hBtnMB, 10, 80, sideW - 20, 45, TRUE);
    MoveWindow(hBtnCB, 10, 135, sideW - 20, 45, TRUE);
    MoveWindow(hBtnHK, 10, 190, sideW - 20, 45, TRUE);
    int listX = sideW + 10;
    int listW = w - listX - 10;
    int listH = h - 80 - 10;
    if (listW > 0 && listH > 0)
    {
        MoveWindow(hListView, listX, 80, listW, listH, TRUE);
    }
    MoveWindow(hBtnAdd, listX, 30, 120, 35, TRUE);
    MoveWindow(hBtnDel, listX + 130, 30, 120, 35, TRUE);
    MoveWindow(hBtnBook, listX + 260, 30, 120, 35, TRUE);
}

void SetupList()
{
    while (ListView_DeleteColumn(hListView, 0));
    SendMessage(hListView, LVM_DELETEALLITEMS, 0, 0);
    LVCOLUMN c;
    c.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    c.fmt = LVCFMT_LEFT;
    if (currentModule == 1)
    {
        c.pszText = (char*)"MA SO"; c.cx = 100; ListView_InsertColumn(hListView, 0, &c);
        c.pszText = (char*)"LOAI MAY BAY"; c.cx = 200; ListView_InsertColumn(hListView, 1, &c);
        c.pszText = (char*)"SO GHE"; c.cx = 100; ListView_InsertColumn(hListView, 2, &c);
        c.pszText = (char*)"LUOT BAY"; c.cx = 100; ListView_InsertColumn(hListView, 3, &c);
    }
    else if (currentModule == 2)
    {
        c.pszText = (char*)"MA CB"; c.cx = 100; ListView_InsertColumn(hListView, 0, &c);
        c.pszText = (char*)"MAY BAY"; c.cx = 100; ListView_InsertColumn(hListView, 1, &c);
        c.pszText = (char*)"NOI DEN"; c.cx = 150; ListView_InsertColumn(hListView, 2, &c);
        c.pszText = (char*)"THOI GIAN"; c.cx = 160; ListView_InsertColumn(hListView, 3, &c);
        c.pszText = (char*)"TINH TRANG"; c.cx = 150; ListView_InsertColumn(hListView, 4, &c);
    }
    else
    {
        c.pszText = (char*)"CCCD"; c.cx = 150; ListView_InsertColumn(hListView, 0, &c);
        c.pszText = (char*)"HO VA TEN"; c.cx = 250; ListView_InsertColumn(hListView, 1, &c);
        c.pszText = (char*)"GIOI TINH"; c.cx = 100; ListView_InsertColumn(hListView, 2, &c);
    }
}

void Refresh()
{
    SendMessage(hListView, LVM_DELETEALLITEMS, 0, 0);
    if (currentModule == 1)
    {
        for (int i = 0; i < dsMB.n; i++)
        {
            LVITEM l = { 0 };
            l.mask = LVIF_TEXT;
            l.iItem = i;
            l.pszText = dsMB.nodes[i]->soHieuMB;
            ListView_InsertItem(hListView, &l);
            ListView_SetItemText(hListView, i, 1, dsMB.nodes[i]->loaiMB);
            ListView_SetItemText(hListView, i, 2, (char*)MyIntToStr(dsMB.nodes[i]->soCho).c_str());
            ListView_SetItemText(hListView, i, 3, (char*)MyIntToStr(dsMB.nodes[i]->soLuotBay).c_str());
        }
    }
    else if (currentModule == 2)
    {
        int i = 0;
        NodeCB* p = headCB;
        while (p)
        {
            LVITEM l = { 0 };
            l.mask = LVIF_TEXT;
            l.iItem = i;
            l.pszText = p->data.maCB;
            ListView_InsertItem(hListView, &l);
            ListView_SetItemText(hListView, i, 1, p->data.maMB);
            ListView_SetItemText(hListView, i, 2, p->data.sanBayDen);
            char d[50];
            FormatDate(p->data.tg, d);
            ListView_SetItemText(hListView, i, 3, d);
            char stt[50];
            if (p->data.trangThai == 0)
            {
                strcpy(stt, "DA HUY");
            }
            else
            {
                int bk = 0;
                for (int k = 0; k < p->data.soCho; k++)
                {
                    if (strlen(p->data.dsVe[k].CCCD) > 0)
                    {
                        bk++;
                    }
                }
                sprintf(stt, "CON VE (%d/%d)", p->data.soCho - bk, p->data.soCho);
            }
            ListView_SetItemText(hListView, i, 4, stt);
            p = p->next;
            i++;
        }
    }
    else
    {
        auto Trav = [&](auto&& s, NodeHK* r) -> void {
            if (!r) return;
            s(s, r->left);
            int idx = ListView_GetItemCount(hListView);
            LVITEM l = { 0 };
            l.mask = LVIF_TEXT;
            l.iItem = idx;
            l.pszText = r->data.CCCD;
            ListView_InsertItem(hListView, &l);
            string f = string(r->data.ho) + " " + string(r->data.ten);
            ListView_SetItemText(hListView, idx, 1, (char*)f.c_str());
            ListView_SetItemText(hListView, idx, 2, r->data.gioiTinh);
            s(s, r->right);
        };
        Trav(Trav, rootHK);
    }
}

HWND hE1, hE2, hE3, hE4, hE5;

LRESULT CALLBACK InputProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_COMMAND && LOWORD(w) == 100)
    {
        char b1[100], b2[100], b3[100], b4[100], b5[100];
        GetWindowText(hE1, b1, 100);
        GetWindowText(hE2, b2, 100);
        GetWindowText(hE3, b3, 100);
        if (currentModule == 1)
        {
            if (!CheckMaMB(b1))
            {
                MessageBox(h, "Ma phai bat dau bang QS/CK/HK", "Loi", 0);
                return 0;
            }
            MayBay mb;
            strcpy(mb.soHieuMB, b1);
            strcpy(mb.loaiMB, b2);
            mb.soCho = atoi(b3);
            mb.soLuotBay = 0;
            if (ThemMB(mb))
            {
                MessageBox(h, "Thanh cong!", "OK", 0);
                DestroyWindow(h);
            }
            else
            {
                MessageBox(h, "Trung ma!", "Loi", 0);
            }
        }
        else if (currentModule == 2)
        {
            GetWindowText(hE4, b4, 100);
            GetWindowText(hE5, b5, 100);
            MayBay* f = NULL;
            for (int i = 0; i < dsMB.n; i++)
            {
                if (strcmp(dsMB.nodes[i]->soHieuMB, b2) == 0)
                {
                    f = dsMB.nodes[i];
                }
            }
            if (!f)
            {
                MessageBox(h, "Khong tim thay May Bay nay", "Loi", 0);
                return 0;
            }
            ChuyenBay cb;
            strcpy(cb.maCB, b1);
            strcpy(cb.maMB, b2);
            strcpy(cb.sanBayDen, b3);
            sscanf(b4, "%d/%d/%d", &cb.tg.ngay, &cb.tg.thang, &cb.tg.nam);
            sscanf(b5, "%d:%d", &cb.tg.gio, &cb.tg.phut);
            cb.trangThai = 1;
            cb.soCho = f->soCho;
            ThemCB(cb);
            MessageBox(h, "Thanh cong!", "OK", 0);
            DestroyWindow(h);
        }
        else
        {
            GetWindowText(hE4, b4, 100);
            if (!CheckCCCD(b1))
            {
                MessageBox(h, "CCCD 12-13 so", "Loi", 0);
                return 0;
            }
            HanhKhach hk;
            strcpy(hk.CCCD, b1);
            strcpy(hk.gioiTinh, b4);
            ChuanHoaTen(b2, b3, hk.ho, hk.ten);
            ThemHK(rootHK, hk);
            MessageBox(h, "Thanh cong!", "OK", 0);
            DestroyWindow(h);
        }
    }
    if (m == WM_CTLCOLORSTATIC)
    {
        SetBkColor((HDC)w, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    if (m == WM_CLOSE)
    {
        DestroyWindow(h);
    }
    return DefWindowProc(h, m, w, l);
}

void ShowInput()
{
    WNDCLASS w = { 0 };
    w.lpfnWndProc = InputProc;
    w.hInstance = GetModuleHandle(NULL);
    w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    w.lpszClassName = "Input";
    RegisterClass(&w);
    HWND h = CreateWindow("Input", "NHAP LIEU", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 250, 200, 400, 350, hMain, 0, 0, 0);
    HFONT f = CreateFontUI(16, 0);
    if (currentModule == 1)
    {
        CreateWindow("Static", "MA MB:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, h, 0, 0, 0);
        hE1 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 10, 150, 20, h, 0, 0, 0);
        CreateWindow("Static", "LOAI:", WS_VISIBLE | WS_CHILD, 10, 40, 100, 20, h, 0, 0, 0);
        hE2 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 40, 150, 20, h, 0, 0, 0);
        CreateWindow("Static", "SO GHE:", WS_VISIBLE | WS_CHILD, 10, 70, 100, 20, h, 0, 0, 0);
        hE3 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 70, 150, 20, h, 0, 0, 0);
    }
    else if (currentModule == 2)
    {
        CreateWindow("Static", "MA CB:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, h, 0, 0, 0);
        hE1 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 10, 150, 20, h, 0, 0, 0);
        CreateWindow("Static", "MA MB:", WS_VISIBLE | WS_CHILD, 10, 40, 100, 20, h, 0, 0, 0);
        hE2 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 40, 150, 20, h, 0, 0, 0);
        CreateWindow("Static", "NOI DEN:", WS_VISIBLE | WS_CHILD, 10, 70, 100, 20, h, 0, 0, 0);
        hE3 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 70, 150, 20, h, 0, 0, 0);
        CreateWindow("Static", "NGAY:", WS_VISIBLE | WS_CHILD, 10, 100, 100, 20, h, 0, 0, 0);
        hE4 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 100, 100, 20, h, 0, 0, 0);
        CreateWindow("Static", "GIO:", WS_VISIBLE | WS_CHILD, 10, 130, 100, 20, h, 0, 0, 0);
        hE5 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 130, 100, 20, h, 0, 0, 0);
    }
    else
    {
        CreateWindow("Static", "CCCD:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, h, 0, 0, 0);
        hE1 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 10, 180, 20, h, 0, 0, 0);
        CreateWindow("Static", "HO DEM:", WS_VISIBLE | WS_CHILD, 10, 40, 100, 20, h, 0, 0, 0);
        hE2 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 40, 180, 20, h, 0, 0, 0);
        CreateWindow("Static", "TEN:", WS_VISIBLE | WS_CHILD, 10, 70, 100, 20, h, 0, 0, 0);
        hE3 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 70, 100, 20, h, 0, 0, 0);
        CreateWindow("Static", "GIOI:", WS_VISIBLE | WS_CHILD, 10, 100, 100, 20, h, 0, 0, 0);
        hE4 = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 100, 80, 20, h, 0, 0, 0);
    }
    HWND btn = CreateWindow("Button", "LUU LAI", WS_VISIBLE | WS_CHILD, 120, 200, 120, 35, h, (HMENU)100, 0, 0);
    SendMessage(btn, WM_SETFONT, (WPARAM)f, TRUE);
}

HWND hE_CCCD;
LRESULT CALLBACK BookProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_COMMAND && LOWORD(w) == 100)
    {
        char cccd[20];
        GetWindowText(hE_CCCD, cccd, 20);
        if (!TimHK(rootHK, cccd))
        {
            MessageBox(h, "CCCD chua co trong DS Hanh Khach. Hay them HK truoc!", "Loi", 0);
            return 0;
        }
        int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
        char maCB[20];
        ListView_GetItemText(hListView, idx, 0, maCB, 20);
        if (DatVe(maCB, cccd))
        {
            MessageBox(h, "Dat Ve Thanh Cong!", "OK", 0);
            DestroyWindow(h);
            Refresh();
        }
        else
        {
            MessageBox(h, "Khong the dat (Het ve/Da dat roi)", "Loi", 0);
        }
    }
    if (m == WM_CLOSE)
    {
        DestroyWindow(h);
    }
    return DefWindowProc(h, m, w, l);
}

void ShowBook()
{
    if (currentModule != 2)
    {
        MessageBox(hMain, "Vui long chon Module Chuyen Bay!", "Loi", 0);
        return;
    }
    int i = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (i == -1)
    {
        MessageBox(hMain, "Chon chuyen bay muon dat!", "Loi", 0);
        return;
    }
    WNDCLASS w = { 0 };
    w.lpfnWndProc = BookProc;
    w.hInstance = GetModuleHandle(NULL);
    w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    w.lpszClassName = "Book";
    RegisterClass(&w);
    HWND h = CreateWindow("Book", "DAT VE", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 300, 300, 350, 150, hMain, 0, 0, 0);
    CreateWindow("Static", "CCCD KHACH:", WS_VISIBLE | WS_CHILD, 10, 20, 100, 20, h, 0, 0, 0);
    hE_CCCD = CreateWindow("Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 20, 180, 20, h, 0, 0, 0);
    HWND btn = CreateWindow("Button", "XAC NHAN", WS_VISIBLE | WS_CHILD, 120, 60, 100, 30, h, (HMENU)100, 0, 0);
}

void OnDel()
{
    int i = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (i == -1)
    {
        return;
    }
    if (currentModule == 1)
    {
        if (MessageBox(hMain, "Xoa May Bay?", 0, MB_YESNO) == IDYES)
        {
            XoaMB(i);
            Refresh();
        }
    }
    else if (currentModule == 2)
    {
        if (MessageBox(hMain, "Huy Chuyen Bay?", 0, MB_YESNO) == IDYES)
        {
            char m[20];
            ListView_GetItemText(hListView, i, 0, m, 20);
            NodeCB* p = headCB;
            while (p && strcmp(p->data.maCB, m) != 0)
            {
                p = p->next;
            }
            if (p)
            {
                p->data.trangThai = 0;
            }
            Refresh();
        }
    }
}

LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_CREATE)
    {
        hFontNormal = CreateFontUI(16, false);
        hFontTitle = CreateFontUI(20, true);
        hFontBold = CreateFontUI(16, true);
        hBrushBg = CreateSolidBrush(COL_BG);
        hBtnMB = CreateWindow("Button", "MAY BAY", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, h, (HMENU)1, 0, 0);
        hBtnCB = CreateWindow("Button", "CHUYEN BAY", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, h, (HMENU)2, 0, 0);
        hBtnHK = CreateWindow("Button", "HANH KHACH", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, h, (HMENU)3, 0, 0);
        hBtnAdd = CreateWindow("Button", "+ THEM MOI", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, h, (HMENU)101, 0, 0);
        hBtnDel = CreateWindow("Button", "- XOA / HUY", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, h, (HMENU)102, 0, 0);
        hBtnBook = CreateWindow("Button", "DAT VE", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, h, (HMENU)103, 0, 0);
        hListView = CreateWindow(WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT, 0, 0, 0, 0, h, (HMENU)99, 0, 0);
        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        SendMessage(hBtnMB, WM_SETFONT, (WPARAM)hFontBold, 1);
        SendMessage(hBtnCB, WM_SETFONT, (WPARAM)hFontBold, 1);
        SendMessage(hBtnHK, WM_SETFONT, (WPARAM)hFontBold, 1);
        SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFontNormal, 1);
        SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hFontNormal, 1);
        SendMessage(hBtnBook, WM_SETFONT, (WPARAM)hFontNormal, 1);
        SendMessage(hListView, WM_SETFONT, (WPARAM)hFontNormal, 1);
        LoadData();
        SetupList();
        Refresh();
    }
    if (m == WM_SIZE)
    {
        ResizeLayout(LOWORD(l), HIWORD(l));
    }
    if (m == WM_CTLCOLORSTATIC)
    {
        SetBkColor((HDC)w, COL_BG);
        return (LRESULT)hBrushBg;
    }
    if (m == WM_COMMAND)
    {
        int id = LOWORD(w);
        if (id >= 1 && id <= 3)
        {
            currentModule = id;
            SetupList();
            Refresh();
        }
        if (id == 101) ShowInput();
        if (id == 102) OnDel();
        if (id == 103) ShowBook();
        if (id == 101) Refresh();
    }
    if (m == WM_DESTROY)
    {
        SaveData();
        PostQuitMessage(0);
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR, int)
{
    INITCOMMONCONTROLSEX x;
    x.dwSize = sizeof(x);
    x.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&x);
    WNDCLASS w = { 0 };
    w.lpfnWndProc = MainProc;
    w.hInstance = hI;
    w.hbrBackground = CreateSolidBrush(COL_BG);
    w.lpszClassName = "Modern";
    w.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&w);
    hMain = CreateWindow("Modern", "HE THONG QUAN LY BAY 2026", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 900, 600, 0, 0, hI, 0);
    if (dsMB.n == 0)
    {
        MayBay mb;
        strcpy(mb.soHieuMB, "QS001");
        strcpy(mb.loaiMB, "Quan Su");
        mb.soCho = 50;
        mb.soLuotBay = 0;
        ThemMB(mb);
    }
    MSG m;
    while (GetMessage(&m, 0, 0, 0))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }
    return 0;
}