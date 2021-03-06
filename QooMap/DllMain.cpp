#define _DEFINE_PTRS
#include <Windows.h>
#include "Offset.h"
#include "D2State.h"
#include "Maphack.h"
#include "AutoMap.h" //runenumber
#include "BuyAction_Info.h"
#include "ScreenHooks.h"
#include "EnteringLevelText.h"
#include <shlwapi.h>
#include <Psapi.h>
#include <locale.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "shlwapi.lib")

HINSTANCE instance;
PatchManager* Patches;
char szPath[260];
Module* pModule = NULL;
bool pAutoParty = false;
bool UserAutomapname = false;
PROCESS_MEMORY_COUNTERS pmc; //GetProcessMemoryInfo 때 사용됨
MEMORYSTATUSEX statusx;
bool d2locale_kor = false;
wchar_t isd2lang_kor[10] = L"";
wchar_t qoomapwindowtitle[256] = L"";

int memoryuse; // 프로세서 메모리 사용량
DWORDLONG Phys; //물리메모리

bool d2extend_test = false;
POINT pt;

LPSTR GetShrinesFolder(LPSTR dest, CHAR ch)
{
	GetModuleFileNameA(instance, dest, MAX_PATH);
	PathRemoveFileSpecA(dest);
	strcat_s(dest,256,"\\image\\");
	return strrchr(dest, ch) + 1;
}

bool ReadProcessBYTES(HANDLE hProcess, DWORD lpAddress, void* buf, int len)
{
   DWORD oldprot, dummy = 0;
   if (VirtualProtectEx(hProcess, (void*) lpAddress, len, PAGE_READWRITE, &oldprot) == 0) return false;
   if (ReadProcessMemory(hProcess, (void*) lpAddress, buf, len, 0) == 0) return false;
   if (VirtualProtectEx(hProcess, (void*) lpAddress, len, oldprot, &dummy) == 0) return false;
   return true;
}

bool StringToBool(const char* str)
{
	switch(tolower(str[0])) {
		case 't': case '1':
			return true;
		case 'f': case '0': default:
			return false;
	}
}

void HighResolution_Test(){

	if (GetUIVarSTUB(UI_CUBE) || GetUIVarSTUB(UI_STASH) || GetUIVarSTUB(UI_INVENTORY) || GetUIVarSTUB(UI_WPMENU)|| GetUIVarSTUB(UI_NPCSHOP)) {
			if (d2extend_ok) {
				GetCursorPos(&pt);
				NewResolution(800,600); 
				d2gfx_10060(2);
				//d2res_extend_width = 0;
				d2res_extend_height = 0;
				d2extend_test = true;
			}
		} //}	
		else {
			if (d2extend_ok) {
				 if (d2extend_test){
					NewResolution(1344,700);
					d2gfx_10060(3);
					//d2res_extend_width = 1344 - 800;
					d2res_extend_height = 100;
				
					}
			}
			if (d2extend_test) {	
				 D2Client::SetUIVar(UI_CHARACTER,0,0);
				 D2Client::SetUIVar(UI_CHARACTER,1,0);
					SetCursorPos(pt.x+272,pt.y);
				 d2extend_test = false; 
			}
		}
}

void AutoTransmuteItems(){
	if (GetUIVarSTUB(UI_CUBE)){
					if(*D2Client::MouseX < 400){
						wchar_t itemdescTemp[2048] = L"";
						ReadProcessBYTES(GetCurrentProcess(), 0x6F9A9E58,itemdescTemp, 2047); //아이템 속성 얻어오기  D2Win::ItemDescBuffer 랑 같음

						if (wcsstr(itemdescTemp,L"[L99][rin]")){ //링일경우
							 if (wcsstr(itemdescTemp,L"+1 to All Skills")) { 
								if (wcsstr(itemdescTemp,L"cast level 49 Supernova")) {
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"+1 to All Skills + Lv 49 Supernova Ring !! ",9);
											Cube_Start = false;
									}
								}
							}
						}

						if (wcsstr(itemdescTemp,L"[L99][amu]")){ //아뮬일경우
							if (wcsstr(itemdescTemp,L"+3 to All Skills")) { 
								if (wcsstr(itemdescTemp,L"cast level 44 Doom")) {
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"+3 All Skills + Lv 44 Doom Amulet !! ",9);
											Cube_Start = false;
									}
								}
								if (wcsstr(itemdescTemp,L"cast level 6 Banish")) {
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"+3 All Skills + Lv 6 Banish Amulet !! ",9);
											Cube_Start = false;
									}
								}

								if (wcsstr(itemdescTemp,L"to Lightning Arena")) {
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"+3 All Skills + Lightning Arena Amulet !! ",9);
											Cube_Start = false;
									}
								}
							}
						}

						if (wcsstr(itemdescTemp,L"Superior")){ //슈페이러일경우
							if (!wcsstr(itemdescTemp,L"Socketed")){ //소켓이 없으면
								if (wcsstr(itemdescTemp,L"60% Enhanced Damage")){
									if (Cube_Start == true) {
												D2Client::PrintGameString(L"+60% Enhanced Damage",3);
												Cube_Start = false;
										}
								}
								if (wcsstr(itemdescTemp,L"60% Enhanced Defense")) {
									if (Cube_Start == true) {
												D2Client::PrintGameString(L"+60% Enhanced Defense",3);
												Cube_Start = false;
										}
								}
							}
						}

						if (wcsstr(itemdescTemp,L"Staff (Sacred)")){ //스테프일경우
							if (wcsstr(itemdescTemp,L"12 to Sorceress Skill Levels")){
								if (wcsstr(itemdescTemp,L"to Avalanche")){
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"+12 Sor skill + Avalanche Staff!!!",8);
											Cube_Start = false;
									}
								}
							}
							if (wcsstr(itemdescTemp,L"to Cold Spell Damage")){
								if (wcsstr(itemdescTemp,L"to Avalanche")){
									if (Cube_Start == true) {
										    D2Client::PrintGameString(L"Cold Spell Damage + Avalanche Staff!!!",8);
											Cube_Start = false;
									}
								}
							}
						}

				}	

				if (Cube_Start == true) {
					Cube_Action(); // 큐브 돌리기
				}
		}
}

void GameLoop() {   // 바로 아래있는 Gameloop_interception이 적용되면 계속 실행이된다.;

		if (ClientState() != ClientStateInGame)  
		     return;//게임일때

		BnetData* pData = *D2Launch::BnData;
		UnitAny *pPlayer = D2Client::GetPlayerUnit();

		HighResolution_Test(); //높이 0 = 800x600 임니다

		//if (lastGameName != pData->szGameName || lastDifficulty != *D2Client::Difficulty){
		if (joingame){
						joingame = false;
						SetForegroundWindow(D2Gfx::GetHwnd());
						char Charname[256] = "";

						ClearAutomapNames();
						jointime = GetTickCount();

						D2Client::PrintGameString(L"ÿc3Vÿc1iÿc4sÿc5iÿc8t ÿc0http://l0veshoo.pusku.com", 1);
						for (int i=0;i<sizeof(QooMapRevealOK);i++) QooMapRevealOK[i] = false;

						wsprintf(Charname,"[ %s ] Myth 모드 - 맵핵 작동중... ver 0.6",pData->szPlayerName);
							//wsprintf(Charname,"Diablo II - Median XL SIGMA 맵핵 작동중... V1 [ %s ]",pData->szPlayerName);
						SetWindowText(D2Gfx::GetHwnd(),Charname);
	
				if(pData->szGameName) //방 이름
					lastGameName = pData->szGameName;
				if(pData->szGamePass) // 방 비번
					lastGamePass = pData->szGamePass;

				inGame = true;
				if (AlwaysTop == false) SetWindowPos(D2Gfx::GetHwnd(),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			}

		if (inGame){
			if (GetProcessMemoryInfo(GetCurrentProcess(),&pmc,sizeof(pmc)) != NULL) memoryuse = pmc.WorkingSetSize / 1024/1024;
			if (memoryuse > 500) EmptyWorkingSet(GetCurrentProcess()); 
			
			if (QooMapRevealOK[pPlayer->dwAct] == false) {
				RevealAct((pPlayer->dwAct+1)); // 각 액트 로드
			}

			if (pAutoParty) AutoParty();

			if (QooMapChicken) {
				int MyLifePercent = (int)((double)(D2Common::GetUnitStat(D2Client::GetPlayerUnit(), 6,0) >> 8)/(double)(D2Common::GetUnitStat(D2Client::GetPlayerUnit(), 7,0) >> 8)*100);
		
				if (!D2Common::IsLevelATown(D2Common::GetRoom1LevelNo(D2Client::GetPlayerUnit()->pPath->pRoom1))){
					if ((unsigned int)MyLifePercent <= QooMapChickenLifePercent){
						if (QooMapChickenTown == true)  {QooMapChickenTown = false; CtrlAction(D2Gfx::GetHwnd()); }
						if (QooMapChickenLeftGame == true) D2Client::ExitGame();
					}
				} else if(QooMapChickenLeftGame !=true) QooMapChickenTown = true;
			}

			if (UserAutomapname == true) EnumerateVectors(); // 오토맵 컴퍼스#

			if (!GetUIVarSTUB(UI_INVENTORY) || GetUIVarSTUB(UI_TRADE)) viewunit = NULL;
		
			if (DrawSetting == true) D2Client::SetUIVar(UI_QUEST,0,0);

			if (AutoEatCorpse == true&& DieOK == true){
					 if (playercorpsex < pPlayer->pPath->xPos+30 && playercorpsex > pPlayer->pPath->xPos-30 && playercorpsey > pPlayer->pPath->yPos-30 && playercorpsex < pPlayer->pPath->xPos+30 ){
					//if (!D2Common::IsLevelATown(D2Common::GetRoom1LevelNo(D2Client::GetPlayerUnit()->pPath->pRoom1)) && !GetUIVarSTUB(UI_INVENTORY)){
						if (!GetUIVarSTUB(UI_INVENTORY)){
							int ItemCount = 0;
							for(UnitAny* pItem = D2Common::GetFirstItemFromInventory(D2Client::GetPlayerUnit()->pInventory); pItem; pItem = D2Common::GetNextItemFromInventory(pItem)) {
								if (pItem->pItemData->InventoryLocation == STORAGE_NULL){
									if (pItem->pItemData->NodePage == NODEPAGE_EQUIP) { //착용템
											ItemCount++;
										}
								}
							}

							if (ItemCount == 0){
								SendGamePacket(TakeCorpse,sizeof(TakeCorpse));
							}
							else DieOK = false;
						}
					
					}
				 }


			if (AlwaysAltDown == 2){ // 알트키 눌렀을시
				UnitAny *SelectUnit = D2Client::GetSelectedUnit();
				D2Client::SetUIVar(UI_ALTDOWN,0,0);
				if (GetUIVarSTUB(UI_CUBE) || GetUIVarSTUB(UI_STASH)) D2Client::SetUIVar(UI_ALTDOWN,1,0);

				if (GetUIVarSTUB(UI_INVENTORY) && *D2Client::MouseX > 400)
					if (GetUIVarSTUB(UI_ALTDOWN))D2Client::SetUIVar(UI_ALTDOWN,1,0);
				if (SelectUnit){
					if (SelectUnit->dwType == UNIT_TYPE_MONSTER ||SelectUnit->dwType == UNIT_TYPE_OBJECT || SelectUnit->dwType == UNIT_TYPE_ROOMTILE)
					if (GetUIVarSTUB(UI_ALTDOWN))D2Client::SetUIVar(UI_ALTDOWN,1,0);
				}
			}
			if (AlwaysAltDown == 1){
				D2Client::SetUIVar(UI_ALTDOWN,0,0);
				if (GetUIVarSTUB(UI_CUBE) || GetUIVarSTUB(UI_STASH)) D2Client::SetUIVar(UI_ALTDOWN,1,0);
				if (GetUIVarSTUB(UI_INVENTORY) && *D2Client::MouseX > 400)
					D2Client::SetUIVar(UI_ALTDOWN,1,0);
			}
		 }
} 

//Interception code!
VOID __declspec(naked) GameLoop_Interception()
{
	__asm
	{
		pushad;
		call GameLoop;
		popad;
 
		pop eax;
		sub esp, 0x20;
		mov [esp + 0xC], ecx;
		push eax;
		ret;
	}
}

void ini_load(){
			strncpy_s(pFile, sizeof(pFile), szPath, sizeof(pFile));
			strcat_s(pFile, "\\QooMap.ini");

			char ini_runenumber[6]; //바로밑 ini에서 읽어올때 사용
			char ini_rareamulet[6];
			char ini_magicjewel[6];
			char ini_rarejewel[6];
			
			char ini_Creepyshrine[6];
			char ini_AbandonedShrine[6];
			char ini_QuietShrine[6];
			char ini_EerieShrine[6];
			char ini_WeirdShrine[6];
			char ini_GloomyShrine[6];
			char ini_TaintedShrine[6];
			char ini_FascinatingShrine[6];
			char ini_OrnateShrine[6];
			char ini_SacredShrine[6];
			char ini_ShimmeringShrine[6];
			char ini_SpiritualShrine[6];
			char ini_MagicalShrine[6];
			char ini_HiddenShrine[6];
			char ini_StoneShrine[6];
			char ini_HealingPotion[6];
			char ini_ManaPotion[6];

			char ini_Amethyst[6];
			char ini_Topaz[6];
			char ini_Sapphire[6];
			char ini_Emerald[6];
			char ini_Diamond[6];
			char ini_Ruby[6];
			char ini_Skull[6];
			char ini_Rainbowstone[6];
			char ini_Onyx[6];
			char ini_Bloodstone[6];
			char ini_Turquoise[6];
			char ini_Amber[6];
			char ini_Ghostly[6];

			char ini_Effect[6];
			char ini_socket3[6];
			char ini_socket4[6];
			char ini_socket5[6];
			char ini_socket6[6];

			char ini_AutoEatCorpse[6];
			char ini_EnemyCorpse[6];
			char ini_MapName[6];
			char ini_QooMapChicken[6];

			char ini_Gold[6];
			char ini_OnlyUnique[6];
			char ini_QooMapChickenTown[6];
			char ini_QooMapChickenLeftGame[6];
			char ini_QooMapChickenLifePercent[6];
			char ini_DropNotify[6];
			char ini_AutoParty[6];
			char ini_UberQuest[6];
			char ini_AutoTransmute[6];
			char ini_RareRing[6];
			char ini_sellprice[6];


			GetPrivateProfileString("MapHack", "RuneNumber", "1", ini_runenumber, 6, pFile);
			GetPrivateProfileString("MapHack", "Rare Amulet", "1", ini_rareamulet, 6, pFile);
			GetPrivateProfileString("MapHack", "Magic Jewel", "1", ini_magicjewel, 6, pFile);
			GetPrivateProfileString("MapHack", "Rare Jewel", "1", ini_rarejewel, 6, pFile);

			GetPrivateProfileString("MapHack", "Perfect Amethyst", "0",ini_Amethyst, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Topaz", "0", ini_Topaz, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Sapphire", "0", ini_Sapphire, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Diamond", "0", ini_Diamond, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Emerald", "0", ini_Emerald, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Ruby", "0", ini_Ruby, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Skull", "0", ini_Skull, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Rainbowstone", "0",ini_Rainbowstone, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Onyx", "0", ini_Onyx, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Bloodstone", "0", ini_Bloodstone, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Turquoise", "0", ini_Turquoise, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Amber", "0", ini_Amber, 6, pFile);
			GetPrivateProfileString("MapHack", "Perfect Ghostly", "0", ini_Ghostly, 6, pFile);

			GetPrivateProfileString("MapHack", "Creepy shrine", "0", ini_Creepyshrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Abandoned Shrine", "0", ini_AbandonedShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Quiet Shrine", "0", ini_QuietShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Eerie Shrine", "0", ini_EerieShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Weird Shrine", "0", ini_WeirdShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Gloomy Shrine", "0", ini_GloomyShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Tainted Shrine", "0", ini_TaintedShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Fascinating Shrine", "0", ini_FascinatingShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Ornate Shrine", "0", ini_OrnateShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Sacred Shrine", "0", ini_SacredShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Shimmering Shrine", "0", ini_ShimmeringShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Spiritual Shrine", "0", ini_SpiritualShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Magical Shrine", "0", ini_MagicalShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Hidden Shrine", "0", ini_HiddenShrine, 6, pFile);
			GetPrivateProfileString("MapHack", "Stone Shrine", "0", ini_StoneShrine, 6, pFile);

			GetPrivateProfileString("MapHack", "Effect", "0", ini_Effect, 6, pFile);
			GetPrivateProfileString("MapHack", "AutoEatCorpse", "1", ini_AutoEatCorpse, 6, pFile);
			GetPrivateProfileString("MapHack", "EnemyCorpse", "0", ini_EnemyCorpse, 6, pFile);
			GetPrivateProfileString("MapHack", "MapName", "1", ini_MapName, 6, pFile);
			GetPrivateProfileString("MapHack", "Gold", "3000", ini_Gold, 6, pFile);
			GetPrivateProfileString("MapHack", "UniqueOnly", "0", ini_OnlyUnique, 6, pFile);

			GetPrivateProfileString("MapHack", "Chicken", "0", ini_QooMapChicken, 6, pFile);
			GetPrivateProfileString("MapHack", "ChickenTown", "1", ini_QooMapChickenTown, 6, pFile);
			GetPrivateProfileString("MapHack", "ChickenLeftGame", "0", ini_QooMapChickenLeftGame, 6, pFile);
			GetPrivateProfileString("MapHack", "ChickenLifePercent", "40", ini_QooMapChickenLifePercent, 6, pFile);
			GetPrivateProfileString("MapHack", "DropNotify", "1", ini_DropNotify, 6, pFile);
			GetPrivateProfileString("MapHack", "AutoParty", "1", ini_AutoParty, 6, pFile);
			GetPrivateProfileString("MapHack", "AutoTransmute", "0", ini_AutoTransmute, 6, pFile);
			GetPrivateProfileString("MapHack", "UberQuest", "0", ini_UberQuest, 6, pFile);
			GetPrivateProfileString("MapHack", "RareRing", "1", ini_RareRing, 6, pFile);

			GetPrivateProfileString("MapHack", "Socket3", "0", ini_socket3, 6, pFile);
			GetPrivateProfileString("MapHack", "Socket4", "0", ini_socket4, 6, pFile);
			GetPrivateProfileString("MapHack", "Socket5", "0", ini_socket5, 6, pFile);
			GetPrivateProfileString("MapHack", "Socket6", "0", ini_socket6, 6, pFile);
			GetPrivateProfileString("MapHack", "SellPrice", "1", ini_sellprice, 6, pFile);

			GetPrivateProfileString("MapHack", "HealingPotion", "0", ini_HealingPotion, 6, pFile);
			GetPrivateProfileString("MapHack", "ManaPotion", "0", ini_ManaPotion, 6, pFile);

			
			runenumber = atoi(ini_runenumber);
			RareAmulet =  StringToBool(ini_rareamulet);
			ViewJewelMagic =  StringToBool(ini_magicjewel);
			ViewJewelRare =  StringToBool(ini_rarejewel);

			CreepyShrine =  StringToBool(ini_Creepyshrine);
			AbandonedShrine = StringToBool(ini_AbandonedShrine);
			QuietShrine =  StringToBool(ini_QuietShrine);
			EerieShrine =  StringToBool(ini_EerieShrine);
			WeirdShrine = StringToBool(ini_WeirdShrine);
			GloomyShrine = StringToBool(ini_GloomyShrine);
			TaintedShrine = StringToBool(ini_TaintedShrine);
			FascinatingShrine = StringToBool(ini_FascinatingShrine);
			OrnateShrine =  StringToBool(ini_OrnateShrine);
			SacredShrine = StringToBool(ini_SacredShrine);
			ShimmeringShrine =  StringToBool(ini_ShimmeringShrine);
			SpiritualShrine = StringToBool(ini_SpiritualShrine);
			MagicalShrine = StringToBool(ini_MagicalShrine);
			HiddenShrine =  StringToBool(ini_HiddenShrine);
			StoneShrine = StringToBool( ini_StoneShrine);

			PerfectAmethyst = StringToBool(ini_Amethyst);
			PerfectTopaz = StringToBool(ini_Topaz);
			PerfectSapphire = StringToBool(ini_Sapphire);
			PerfectEmerald = StringToBool(ini_Emerald);
			PerfectRuby = StringToBool(ini_Ruby);
			PerfectDiamond = StringToBool(ini_Diamond);
			PerfectSkull = StringToBool(ini_Skull);
			PerfectRainbowStone = StringToBool(ini_Rainbowstone);
			PerfectOnyx = StringToBool(ini_Onyx);
			PerfectBloodStone = StringToBool(ini_Bloodstone);
			PerfectTurquoise = StringToBool(ini_Turquoise);
			PerfectAmber = StringToBool(ini_Amber);
			PerfectGhostly = StringToBool(ini_Ghostly);

			socket3 = StringToBool(ini_socket3);
			socket4 = StringToBool(ini_socket4);
			socket5 = StringToBool(ini_socket5);
			socket6 = StringToBool(ini_socket6);

			healingpotion = StringToBool(ini_HealingPotion);
			manapotion = StringToBool(ini_ManaPotion);

			AutoEatCorpse = StringToBool(ini_AutoEatCorpse);
			QooMapChicken = StringToBool(ini_QooMapChicken);
			EffectView = StringToBool(ini_Effect);
			viewCorpse = StringToBool(ini_EnemyCorpse);
			*D2Client::NoPickup = true;
			UserAutomapname = StringToBool(ini_MapName);
			ViewGold_Value = atoi(ini_Gold);
			QooMapChickenTown = StringToBool(ini_QooMapChickenTown);
			QooMapChickenLeftGame = StringToBool(ini_QooMapChickenLeftGame);
			QooMapChickenLifePercent = atoi(ini_QooMapChickenLifePercent);
			DropNotify = StringToBool(ini_DropNotify);
			pAutoParty = StringToBool(ini_AutoParty);
			UberQuest = StringToBool(ini_UberQuest);
			AutoTransmute = StringToBool(ini_AutoTransmute);
			RareRing = StringToBool(ini_RareRing);
			SellPriceCheck = StringToBool(ini_sellprice);
			
			CHAR myFileName[256];

			MenuAllUnique = atoi(ini_OnlyUnique);
			if (MenuAllUnique == 0){
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "allitem.bmp");
				MenuAllUniqueItem = 0x13; //노랑색
				OnlyUniqueOK = false;
			}else{
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "allunique.bmp");
				MenuAllUniqueItem = 0x0e;
				OnlyUniqueOK = true;
			}


			/*
			if (MenuAllUnique == 0){
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "sacred.bmp");
				MenuAllUniqueItem = 0x0e;
				MenuAllUnique = 1;
				OnlyUniqueOK = true;}
			else if (MenuAllUnique == 1){
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "allunique.bmp");
				MenuAllUniqueItem = 0x0e;
				MenuAllUnique =2;
				OnlyUniqueOK = true;}
			else if (MenuAllUnique == 2){
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "tieroff.bmp");
				MenuAllUniqueItem = 0x0c; //노랑색
				MenuAllUnique =3;
				OnlyUniqueOK = false;}
			else {
				wsprintfA(GetShrinesFolder(myFileName, '\\'), "allitem.bmp");
				MenuAllUniqueItem = 0x13; //노랑색
				OnlyUniqueOK = false;
				MenuAllUnique= 0;
			}
			*/
			ItemConfCellFile = InitCellFile(LoadBMPCellFile(myFileName)); //아이템 위치
}

//Dllmain!
BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason, LPVOID lpvReserved) {
	switch(dwReason) {
		case DLL_PROCESS_ATTACH: 
			instance = hInstance;
 
		    OffsetManager::Instance()->DefineOffsets();

			Patches = new PatchManager;
			Patches->InstallPatches();
			
			// 한글판 체크
			wsprintfW(isd2lang_kor,L"%s",D2Lang::GetLocaleText(1082));
			if (wcsstr(isd2lang_kor,L"안녕")) {
				d2locale_kor = true; // 한글판일경우 1082 = "안녕"
				setlocale(LC_ALL,"korean");} // 한글 표현할대 사용. #include <locale.h> 사용

			if (wcsstr(D2Lang::GetLocaleText(23332),L"Ultimative"))
					wsprintfW(qoomapwindowtitle,L"Ultimative");
			else wsprintfW(qoomapwindowtitle,L"SIGMA");
		

			if(D2Gfx::GetHwnd() != INVALID_HANDLE_VALUE)
			OldWndProc = (WNDPROC)SetWindowLongA(D2Gfx::GetHwnd(), GWL_WNDPROC, (LONG)WndProcHandler);

			if(lpvReserved != NULL) {
				pModule = (Module*)lpvReserved;
				if(!pModule)
					return FALSE;
				strcpy_s(szPath, MAX_PATH, pModule->szPath);

			} else {
				GetModuleFileNameA((HMODULE)hInstance, szPath, MAX_PATH);
				PathRemoveFileSpecA(szPath);
				strcat_s(szPath, MAX_PATH, "\\");
			}
			ini_load(); // ini 로드
			CHAR FileName[256];

			wsprintfA(GetShrinesFolder(FileName, '\\'), "bomb.bmp");
			BombCellFile = InitCellFile(LoadBMPCellFile(FileName));

			wsprintfA(GetShrinesFolder(FileName, '\\'), "missile.bmp");
			MissileCellFile = InitCellFile(LoadBMPCellFile(FileName));

			wsprintfA(GetShrinesFolder(FileName, '\\'), "cross.bmp");
			CrossCellFile = InitCellFile(LoadBMPCellFile(FileName));

			wsprintfA(GetShrinesFolder(FileName, '\\'), "item.bmp");
			ItemCellFile = InitCellFile(LoadBMPCellFile(FileName));

			wsprintfA(GetShrinesFolder(FileName, '\\'), "chicken.bmp");
			ChickenCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "effectview.bmp");
			EffectViewCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "scrollmap.bmp");
			ScrollMapCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "mapname.bmp");
			MapNameCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "dropnotice.bmp");
			DropNoticeCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "highres.bmp");
			HighresCellFile = InitCellFile(LoadBMPCellFile(FileName));//아이템 위치
			wsprintfA(GetShrinesFolder(FileName, '\\'), "setting.bmp");
			SettingCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치

			wsprintfA(GetShrinesFolder(FileName, '\\'), "board.bmp");
			BoardCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치

			wsprintfA(GetShrinesFolder(FileName, '\\'), "autotransmute.bmp");
			AutoTransmuteCellFile = InitCellFile(LoadBMPCellFile(FileName)); //아이템 위치

			// 맵 초기화 안하면 싱글 실행시 오류
			AutomapNames.reserve(6);
			AutomapNames.resize(6);

			joingame = true;  // 초기화 안하면 첨 실행시 실행안됨

			break;

		case DLL_PROCESS_DETACH:
		ClearAutomapNames();
		DestroyVectors();
		SetWindowLongA(D2Gfx::GetHwnd(), GWL_WNDPROC, (LONG)OldWndProc);
		Patches->RemovePatches();
		delete Patches;

		OffsetManager::Instance()->CleanUp();
		break;
		}
	
	return true;
	}

