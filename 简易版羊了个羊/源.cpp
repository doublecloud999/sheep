#include <iostream>
#include <easyx.h>
#include <time.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <random>
#include <map> 

#define KUAI_W 60
#define BK_W 640
#define BK_H 800

using namespace std;

class IM {
public:
    IMAGE bk;
    IMAGE im[13];
    IMAGE dark_im[13];//暗
    void init();
} Im;

class Kuai {
public:
    Kuai(int x, int y, int image_id);
    void Draw();
    bool Contral(ExMessage msg);
    int x;
    int y;
    int image_id;
    bool isclock;
    bool isexist;
    bool isCovered;
};

class Game {
public:
    Game();
    void initArr();
    void draw();
    void updata();
    void KuaiUpdata();
    void sameRemove();
    void judgeEnd();
    void resetGame();

    list<Kuai> Kuai_arr;
    list<Kuai> YiXuan;
    int grade;
    bool needRecheck;
    int slot_y;
    
    bool gameStarted;
    bool gameOver;
    
private:
    bool isTopLayer(const Kuai& k);//
} GAME;

int main() {
    srand((unsigned int)time(NULL));
    initgraph(BK_W, BK_H, EX_SHOWCONSOLE);
    Im.init();
    GAME.initArr();
    BeginBatchDraw();
    while (1) {
        GAME.updata();
        GAME.draw();
        FlushBatchDraw();
    }
    EndBatchDraw();
    return 0;
}

void IM::init() {
    loadimage(&bk, "./images/bk.jpg", BK_W, BK_H);
    
    for (int i = 0; i < 13; i++) {
        char s[132] = {0};
        sprintf(s, "./images/%d.jpg", i + 1);
        loadimage(im + i, s, KUAI_W, KUAI_W);
        
        dark_im[i] = im[i];
        DWORD* dst = GetImageBuffer(&dark_im[i]);
        int width = dark_im[i].getwidth();
        int height = dark_im[i].getheight();
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                COLORREF color = dst[y * width + x];
                int r = GetRValue(color) * 0.5;
                int g = GetGValue(color) * 0.5;
                int b = GetBValue(color) * 0.5;
                dst[y * width + x] = RGB(r, g, b);
            }
        }
    }
}

Kuai::Kuai(int x, int y, int image_id) : x(x), y(y), image_id(image_id) {
    isclock = true;
    isexist = true;
    isCovered = false;
}

void Kuai::Draw() {
    putimage(x - KUAI_W/2, y - KUAI_W/2, Im.im + image_id);
}

bool Kuai::Contral(ExMessage msg) {
    if (msg.x >= x - KUAI_W/2 && msg.x <= x + KUAI_W/2 && 
        msg.y >= y - KUAI_W/2 && msg.y <= y + KUAI_W/2 && isclock) {
        if (msg.message == WM_LBUTTONDOWN) {
            return true;
        }
    }
    return false;
}

Game::Game() {
    grade = 1;
    needRecheck = true;
    slot_y = 680;
    gameStarted = false;
    gameOver = false;
}

void Game::resetGame() {
    gameStarted = true;
    gameOver = false;
    grade = 1;
    Kuai_arr.clear();
    YiXuan.clear();
    initArr();
}

bool Game::isTopLayer(const Kuai& k) {
    for (const auto& other : Kuai_arr) {
        if (&k != &other) {
            int dx = abs(other.x - k.x);
            int dy = other.y - k.y;
            
            if (dy < 0) {
                int overlapX = max(0, KUAI_W - dx);
                int overlapY = max(0, KUAI_W + dy);
                int overlap = overlapX * overlapY;
                
                if (overlap > 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

void Game::initArr() {
    Kuai_arr.clear();
    YiXuan.clear();
    needRecheck = true;
    
    if (grade == 1) {
        vector<pair<int, int>> positions = {
            {160, 160}, {320, 160}, {480, 160},
            {160, 320}, {320, 320}, {480, 320},
            {160, 480}, {320, 480}, {480, 480}
        };
        
        vector<int> image_ids;
        for (int i = 0; i < 3; i++) {
            image_ids.push_back(0);
            image_ids.push_back(1);
            image_ids.push_back(2);
        }
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(image_ids.begin(), image_ids.end(), g);
        
        for (int i = 0; i < 9; i++) {
            int x = positions[i].first;
            int y = positions[i].second;
            int image_id = image_ids[i];
            Kuai_arr.push_back(Kuai(x, y, image_id));
        }
    }
    else if (grade == 2) {
        vector<vector<pair<int, int>>> layers = {
            {{170, 190}, {290, 190}, {350, 190}, {470, 190},
             {170, 250}, {470, 250},
             {200, 310}, {260, 310}, {320, 310}, {380, 310}, {440, 310},
             {200, 370}, {260, 370}, {320, 370}, {380, 370}, {440, 370},
             {170, 430}, {470, 430},
             {170, 490}, {470, 490}, {290, 490}, {350, 490},
             {290, 550}, {350, 550}},
            
            {{320, 250}, 
             {260, 310}, {380, 310},
             {260, 370}, {380, 370},
             {320, 430}},
            
            {{320, 310}, {320, 370}}
        };
        
        int temp_arr[13] = {0};
        
        for (auto &layer : layers) {
            for (auto &pos : layer) {
                int temp = rand() % 13;
                temp_arr[temp]++;
                Kuai_arr.push_back(Kuai(pos.first, pos.second, temp));
            }
        }
        
        for (int i = 0; i < 50; i++) {
            int temp = rand() % 13;
            temp_arr[temp]++;
            Kuai_arr.push_back(Kuai(100 + rand() % 22 * 20, 160 + rand() % 20 * 20, temp));
        }
        
        for (int i = 0; i < 13; i++) {
            while (temp_arr[i] % 3 != 0) {
                temp_arr[i]++;
                Kuai_arr.push_back(Kuai(100 + rand() % 22 * 20, 160 + rand() % 20 * 20, i));
            }
        }
    }
}

void Game::draw() {
    putimage(0, 0, &Im.bk);
   
    for (auto& kuai : Kuai_arr) {
        if (kuai.isCovered) {
            putimage(kuai.x - KUAI_W/2, kuai.y - KUAI_W/2, &Im.dark_im[kuai.image_id]);
        }
    }
    
    for (auto& kuai : Kuai_arr) {
        if (!kuai.isCovered) {
            kuai.Draw();
        }
    }
    
    // 将木槽中的图片按种类分组显示
    if (gameStarted && !gameOver) {
        // 按种类分组
        map<int, vector<Kuai>> groupedImages;
        for (const auto& kuai : YiXuan) {
            groupedImages[kuai.image_id].push_back(kuai);
        }
        
        // 按照种类顺序绘制
        int x_offset = 70;
        for (const auto& group : groupedImages) {
            for (const auto& kuai : group.second) {
                int x = x_offset;
                int y = slot_y;
                putimage(x - KUAI_W/2, y - KUAI_W/2, Im.im + kuai.image_id);
                x_offset += KUAI_W + 20;
            }
        }
    }
    
    // 开始按钮 (添加浅蓝背景)
    if (!gameStarted) {
        // 添加半透明浅蓝背景覆盖整个窗口
        setfillcolor(LIGHTBLUE); // 浅蓝色带透明度
        solidrectangle(0, 0, BK_W, BK_H);
        
        // 绘制按钮和文字
        setfillcolor(RGB(100, 200, 100));
        fillroundrect(BK_W/2 - 100, BK_H/2 - 40, BK_W/2 + 100, BK_H/2 + 40, 10, 10);
        settextcolor(WHITE);
        settextstyle(40, 0, "黑体");
        outtextxy(BK_W/2 - textwidth("START")/2, BK_H/2 - 15, "START");
    }
    
    // 游戏结束
    if (gameOver) {
        // 黑色半透明背景
        setfillcolor(LIGHTBLUE);
        solidrectangle(0, 0, BK_W, BK_H);
        
        settextcolor(WHITE);
        settextstyle(80, 0, "黑体");
        outtextxy(BK_W/2 - textwidth("LOSE!")/2, BK_H/3, "LOSE!");
        
        setfillcolor(RGB(200, 100, 100));
        fillroundrect(BK_W/2 - 100, BK_H/2 - 40, BK_W/2 + 100, BK_H/2 + 40, 10, 10);
        settextcolor(WHITE);
        settextstyle(40, 0, "黑体");
        outtextxy(BK_W/2 - textwidth("RESET")/2, BK_H/2 - 15, "RESET");
    }
}


void Game::updata() {
    if (gameStarted && !gameOver && needRecheck) {
        KuaiUpdata();
        needRecheck = false;
    }
    
    if (gameStarted && !gameOver) {
        sameRemove();
        judgeEnd();
    }
    
    ExMessage msg;
    if (peekmessage(&msg, EX_MOUSE)) {
        if (!gameStarted && msg.message == WM_LBUTTONDOWN) {
            if (msg.x >= BK_W/2 - 100 && msg.x <= BK_W/2 + 100 &&
                msg.y >= BK_H/2 - 40 && msg.y <= BK_H/2 + 40) {
                gameStarted = true;
                return;
            }
        }
        
        if (gameOver && msg.message == WM_LBUTTONDOWN) {
            if (msg.x >= BK_W/2 - 100 && msg.x <= BK_W/2 + 100 &&
                msg.y >= BK_H/2 - 40 && msg.y <= BK_H/2 + 40) {
                resetGame();
                return;
            }
        }
        
        if (gameStarted && !gameOver) {
            list<list<Kuai>::iterator> topLayers;
            for (auto it = Kuai_arr.begin(); it != Kuai_arr.end(); ++it) {
                if (!it->isCovered && it->isclock) {
                    topLayers.push_back(it);
                }
            }
            
            for (auto rit = topLayers.rbegin(); rit != topLayers.rend(); ++rit) {
                if ((*rit)->Contral(msg)) {
                    YiXuan.push_back(Kuai(0, 0, (*rit)->image_id));
                    Kuai_arr.erase(*rit);
                    needRecheck = true;
                    break;
                }
            }
        }
    }
}

void Game::KuaiUpdata() {
    for (auto& k : Kuai_arr) {
        k.isCovered = true;
        k.isclock = false;
    }
    
    for (auto& k : Kuai_arr) {
        if (isTopLayer(k)) {
            k.isCovered = false;
            k.isclock = true;
        }
    }
}

void Game::sameRemove() {
    int id_arr[13] = {0};
    
    for (auto& kuai : YiXuan) {
        id_arr[kuai.image_id]++;
    }
    
    bool removed = false;
    
    for (int i = 0; i < 13; i++) {
        while (id_arr[i] >= 3) {
            int count = 0;
            for (auto it = YiXuan.begin(); it != YiXuan.end(); ) {
                if (it->image_id == i && count < 3) {
                    it = YiXuan.erase(it);
                    id_arr[i]--;
                    count++;
                    removed = true;
                } else {
                    ++it;
                }
            }
        }
    }
    
    if (removed) needRecheck = true;
}

void Game::judgeEnd() {
    if (Kuai_arr.empty() && YiXuan.empty()) {
        grade++;
        if (grade <= 2) {
            initArr();
        }
        else {
            setfillcolor(WHITE);
            setlinecolor(BLACK);
            settextcolor(BLACK);
            setbkmode(TRANSPARENT);
            settextstyle(80, 0, "黑体");
            fillrectangle(100, 200, 540, 300);
            outtextxy(130, 210, "Win!");
            FlushBatchDraw();
            Sleep(5000);
            exit(0);
        }
    }
    
    if (YiXuan.size() >= 7) {
        gameOver = true;
        needRecheck = true;
    }
}