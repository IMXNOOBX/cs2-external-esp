#include "common.hpp"
#include "Weapon.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include "assets/fonts/WeaponIcons.h"

bool Weapon::Update() {
	auto p = Engine::GetProcess();
    auto client = Engine::GetClient();
	if (!p)
		return false;

    if (!entity_list)
        return false;

    // Similar to Player::GetPawn()
    uintptr_t bucket_ptr = p->read<uintptr_t>(entity_list + 0x10 + 0x8 * ((slot_index & 0x7FFF) >> 9));
    if (!bucket_ptr)
        return false;

    uintptr_t weapon_ptr = p->read<uintptr_t>(bucket_ptr + 0x70 * (slot_index & 0x1FF));
	if (!weapon_ptr)
		return false;

	this->item_index = p->read<short>(weapon_ptr + offsets::pawn::m_AttributeManager + offsets::pawn::m_Item + offsets::pawn::m_iItemDefinitionIndex);

	if (!this->item_index)
		return false;

    this->name = ToString();	
    this->icon = ToIcon();
    this->ammo = p->read<int32_t>(weapon_ptr + offsets::pawn::m_iClip1);
    this->is_reloading = p->read<bool>(weapon_ptr + offsets::pawn::m_bInReload);

	return true;
}

const char* Weapon::ToIcon() const
{
    switch (this->item_index)
    {
    case weapon_ssg08:               return WeaponIcons::SSG08;
    case weapon_xm1014:              return WeaponIcons::XM1014;
    case weapon_sawedoff:            return WeaponIcons::SAWEDOFF;
    case weapon_mag7:                return WeaponIcons::MAG7;
    case weapon_nova:                return WeaponIcons::NOVA;
    case weapon_negev:               return WeaponIcons::NEGEV;
    case weapon_m249:                return WeaponIcons::M249;

    case weapon_taser:               return WeaponIcons::TASER;
    case weapon_flashbang:           return WeaponIcons::FLASHBANG;
    case weapon_frag_grenade:        return WeaponIcons::FRAG_GRENADE;
    case weapon_hegrenade:           return WeaponIcons::FRAG_GRENADE;
    case weapon_smokegrenade:        return WeaponIcons::SMOKEGRENADE;
    case weapon_molotov:             return WeaponIcons::MOLOTOV;
    case weapon_decoy:               return WeaponIcons::DECOY;
    case weapon_incgrenade:          return WeaponIcons::INCGRENADE;
    case weapon_c4:                  return WeaponIcons::C4;

    case item_kevlar:               return WeaponIcons::KEVLAR;
    case item_assaultsuit:          return WeaponIcons::ASSAULTSUIT;
    case item_cutters:              return WeaponIcons::CUTTERS;
    case item_defuser:              return WeaponIcons::DEFUSER;

    case -1:                        return WeaponIcons::SCOPE;
    case -2:                        return WeaponIcons::LIGHT;
    case -3:                        return WeaponIcons::WRENCH;
    case -4:                        return WeaponIcons::GEARS;


    case weapon_deagle:               return WeaponIcons::DEAGLE;
    case weapon_elite:                return WeaponIcons::ELITE;
    case weapon_fiveseven:            return WeaponIcons::FIVESEVEN;
    case weapon_glock:                return WeaponIcons::GLOCK;
    case weapon_hkp2000:              return WeaponIcons::HKP2000;
    case weapon_p250:                 return WeaponIcons::P250;
    case weapon_usp_silencer:         return WeaponIcons::USP_SILENCER;
    case weapon_tec9:                 return WeaponIcons::TEC9;
    case weapon_cz75a:                return WeaponIcons::CZ75A;
    case weapon_revolver:             return WeaponIcons::REVOLVER;

    case weapon_mac10:                return WeaponIcons::MAC10;
    case weapon_ump45:                return WeaponIcons::UMP45;
    case weapon_bizon:                return WeaponIcons::BIZON;
    case weapon_mp7:                  return WeaponIcons::MP7;
    case weapon_mp9:                  return WeaponIcons::MP9;
    case weapon_p90:                  return WeaponIcons::P90;

    case weapon_galilar:              return WeaponIcons::GALILAR;
    case weapon_famas:                return WeaponIcons::FAMAS;
    case weapon_m4a1:                 return WeaponIcons::M4A1;
    case weapon_m4a1_silencer:        return WeaponIcons::M4A1_SILENCER;
    case weapon_aug:                  return WeaponIcons::AUG;
    case weapon_sg556:                return WeaponIcons::SG556;
    case weapon_ak47:                 return WeaponIcons::AK47;

    case weapon_g3sg1:                return WeaponIcons::G3SG1;
    case weapon_scar20:               return WeaponIcons::SCAR20;
    case weapon_awp:                  return WeaponIcons::AWP;

    case weapon_knife_cord:           return WeaponIcons::KNIFE_CORD;
    case weapon_knife_flip:           return WeaponIcons::KNIFE_FLIP;
    case weapon_knife_gut:            return WeaponIcons::KNIFE_GUT;
    case weapon_knife_karambit:       return WeaponIcons::KNIFE_KARAMBIT;
    case weapon_knife_bayonet:        return WeaponIcons::KNIFE_BAYONET;
    case weapon_knife_tactical:       return WeaponIcons::KNIFE_TACTICAL;
    case weapon_knife_survival_bowie: return WeaponIcons::KNIFE_BOWIE;
    case weapon_knife_butterfly:      return WeaponIcons::KNIFE_BUTTERFLY;
    case weapon_knife_push:           return WeaponIcons::KNIFE_PUSH;
    case weapon_knife_falchion:       return WeaponIcons::KNIFE_FALCHION;
    case weapon_knife_t:              return WeaponIcons::KNIFE_T;
    case weapon_knife:                return WeaponIcons::KNIFE_CT;
    case weapon_knife_m9_bayonet:     return WeaponIcons::KNIFE_M9_BAYONET;
    case weapon_knife_canis:          return WeaponIcons::KNIFE_CANIS;
    case weapon_knife_css:            return WeaponIcons::KNIFE_CSS;
    case weapon_knife_ghost:          return WeaponIcons::KNIFE_T;
    case weapon_knife_gypsy_jackknife:return WeaponIcons::KNIFE_GYPSY_JACKKNIFE;
    case weapon_knife_ursus:          return WeaponIcons::KNIFE_URSUS;
    case weapon_knife_stiletto:       return WeaponIcons::KNIFE_STILLETO;
    case weapon_knife_widowmaker:     return WeaponIcons::KNIFE_WIDOWMAKER;
    case weapon_knife_skeleton:       return WeaponIcons::KNIFE_SKELETON;
    case weapon_knife_kukri:          return WeaponIcons::KNIFE_KUKRI;
    case weapon_knife_outdoor:        return WeaponIcons::KNIFE_OUTDOOR;
    default:                          return WeaponIcons::unknown;
    }
}

const char* Weapon::ToString() const
{
    switch (this->item_index)
    {
    case weapon_deagle: return "Deagle";
    case weapon_elite: return "Dual Berettas";
    case weapon_fiveseven: return "Five-Seven";
    case weapon_glock: return "Glock-18";
    case weapon_ak47: return "AK-47";
    case weapon_aug: return "AUG";
    case weapon_awp: return "AWP";
    case weapon_famas: return "FAMAS";
    case weapon_g3sg1: return "G3SG1";
    case weapon_galilar: return "Galil AR";
    case weapon_m249: return "M249";
    case weapon_m4a1: return "M4A1";
    case weapon_mac10: return "MAC-10";
    case weapon_p90: return "P90";
    case weapon_zone_repulsor: return "zone_repulsor";
    case weapon_mp5sd: return "MP5-SD";
    case weapon_ump45: return "UMP-45";
    case weapon_xm1014: return "XM1014";
    case weapon_bizon: return "PP-Bizon";
    case weapon_mag7: return "MAG-7";
    case weapon_negev: return "Negev";
    case weapon_sawedoff: return "Sawed-Off";
    case weapon_tec9: return "Tec-9";
    case weapon_taser: return "Zeus x27";
    case weapon_hkp2000: return "P2000";
    case weapon_mp7: return "MP7";
    case weapon_mp9: return "MP9";
    case weapon_nova: return "Nova";
    case weapon_p250: return "P250";
    case weapon_shield: return "shield";
    case weapon_scar20: return "SCAR-20";
    case weapon_sg556: return "SG 556";
    case weapon_ssg08: return "SSG 08";
    case weapon_knifegg: return "knifegg";
    case weapon_knife: return "Knife";
    case weapon_flashbang: return "Flashbang";
    case weapon_hegrenade: return "HE Grenade";
    case weapon_smokegrenade: return "Smoke Grenade";
    case weapon_molotov: return "Molotov";
    case weapon_decoy: return "Decoy";
    case weapon_incgrenade: return "Incendiary Grenade";
    case weapon_c4: return "C4";
    case item_kevlar: return "Kevlar";
    case item_assaultsuit: return "Kevlar with helmet";
    case item_heavyassaultsuit: return "heavyassaultsuit";
    case item_nvg: return "nvg";
    case item_defuser: return "Defuser";
    case item_cutters: return "cutters";
    case weapon_healthshot: return "Healthshot";
    case weapon_knife_t: return "Knife";
    case weapon_m4a1_silencer: return "M4A1-S";
    case weapon_usp_silencer: return "USP-S";
    case weapon_cz75a: return "CZ75A";
    case weapon_revolver: return "Revolver";
    case weapon_tagrenade: return "tagrenade";
    case weapon_fists: return "Fists";
    case weapon_breachcharge: return "breachcharge";
    case weapon_tablet: return "tablet";
    case weapon_melee: return "melee";
    case weapon_axe: return "axe";
    case weapon_hammer: return "hammer";
    case weapon_spanner: return "spanner";
    case weapon_knife_ghost: return "knife_ghost";
    case weapon_firebomb: return "firebomb";
    case weapon_diversion: return "diversion";
    case weapon_frag_grenade: return "Grenade";
    case weapon_snowball: return "snowball";
    case weapon_bumpmine: return "bumpmine";
    case weapon_knife_bayonet: return "Bayonet";
    case weapon_knife_css: return "Classic Knife";
    case weapon_knife_flip: return "Flip Knife";
    case weapon_knife_gut: return "Gut Knife";
    case weapon_knife_karambit: return "Karambit";
    case weapon_knife_m9_bayonet: return "M9 Bayonet";
    case weapon_knife_tactical: return "Huntsman Knife";
    case weapon_knife_falchion: return "Falchion Knife";
    case weapon_knife_survival_bowie: return "Bowie Knife";
    case weapon_knife_butterfly: return "Butterfly Knife";
    case weapon_knife_push: return "Shadow Daggers";
    case weapon_knife_cord: return "Paracord Knife";
    case weapon_knife_canis: return "Survival Knife";
    case weapon_knife_ursus: return "Ursus Knife";
    case weapon_knife_gypsy_jackknife: return "Navaja Knife";
    case weapon_knife_outdoor: return "Nomad Knife";
    case weapon_knife_stiletto: return "Stiletto Knife";
    case weapon_knife_widowmaker: return "Talon Knife";
    case weapon_knife_skeleton: return "Skeleton Knife";
    case weapon_knife_kukri: return "Kukri Knife";
    default: return "unknown";
    }
}