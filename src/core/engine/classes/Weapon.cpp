#include "common.hpp"
#include "Weapon.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include "Weapons.hpp"

bool Weapon::Update() {
	auto p = Engine::GetProcess();
	if (!p)
		return false;

	uintptr_t weapon_ptr = p->read<uintptr_t>(
		le + (8 * ((index & 0x7FFF) >> 9)) + 0x70 * (index & 0x1FF));
	
	if (!weapon_ptr)
		return false;

	this->item_index = p->read<short>(weapon_ptr + offsets::pawn::m_AttributeManager + offsets::pawn::m_Item + offsets::pawn::m_iItemDefinitionIndex);
	if (!this->item_index)
		return false;

    this->weapon_name = ToString();
	
	return true;
}

const char* Weapon::ToString() const
{
    switch (this->item_index)
    {
    case weapon_deagle: return "deagle";
    case weapon_elite: return "elite";
    case weapon_fiveseven: return "fiveseven";
    case weapon_glock: return "glock";
    case weapon_ak47: return "ak47";
    case weapon_aug: return "aug";
    case weapon_awp: return "awp";
    case weapon_famas: return "famas";
    case weapon_g3sg1: return "g3sg1";
    case weapon_galilar: return "galilar";
    case weapon_m249: return "m249";
    case weapon_m4a1: return "m4a1";
    case weapon_mac10: return "mac10";
    case weapon_p90: return "p90";
    case weapon_zone_repulsor: return "zone_repulsor";
    case weapon_mp5sd: return "mp5sd";
    case weapon_ump45: return "ump45";
    case weapon_xm1014: return "xm1014";
    case weapon_bizon: return "bizon";
    case weapon_mag7: return "mag7";
    case weapon_negev: return "negev";
    case weapon_sawedoff: return "sawedoff";
    case weapon_tec9: return "tec9";
    case weapon_taser: return "taser";
    case weapon_hkp2000: return "hkp2000";
    case weapon_mp7: return "mp7";
    case weapon_mp9: return "mp9";
    case weapon_nova: return "nova";
    case weapon_p250: return "p250";
    case weapon_shield: return "shield";
    case weapon_scar20: return "scar20";
    case weapon_sg556: return "sg556";
    case weapon_ssg08: return "ssg08";
    case weapon_knifegg: return "knifegg";
    case weapon_knife: return "knife";
    case weapon_flashbang: return "flashbang";
    case weapon_hegrenade: return "hegrenade";
    case weapon_smokegrenade: return "smokegrenade";
    case weapon_molotov: return "molotov";
    case weapon_decoy: return "decoy";
    case weapon_incgrenade: return "incgrenade";
    case weapon_c4: return "c4";
    case item_kevlar: return "kevlar";
    case item_assaultsuit: return "assaultsuit";
    case item_heavyassaultsuit: return "heavyassaultsuit";
    case item_nvg: return "nvg";
    case item_defuser: return "defuser";
    case item_cutters: return "cutters";
    case weapon_healthshot: return "healthshot";
    case weapon_knife_t: return "knife_t";
    case weapon_m4a1_silencer: return "m4a1_silencer";
    case weapon_usp_silencer: return "usp_silencer";
    case weapon_cz75a: return "cz75a";
    case weapon_revolver: return "revolver";
    case weapon_tagrenade: return "tagrenade";
    case weapon_fists: return "fists";
    case weapon_breachcharge: return "breachcharge";
    case weapon_tablet: return "tablet";
    case weapon_melee: return "melee";
    case weapon_axe: return "axe";
    case weapon_hammer: return "hammer";
    case weapon_spanner: return "spanner";
    case weapon_knife_ghost: return "knife_ghost";
    case weapon_firebomb: return "firebomb";
    case weapon_diversion: return "diversion";
    case weapon_frag_grenade: return "frag_grenade";
    case weapon_snowball: return "snowball";
    case weapon_bumpmine: return "bumpmine";
    case weapon_knife_bayonet: return "knife_bayonet";
    case weapon_knife_css: return "knife_css";
    case weapon_knife_flip: return "knife_flip";
    case weapon_knife_gut: return "knife_gut";
    case weapon_knife_karambit: return "knife_karambit";
    case weapon_knife_m9_bayonet: return "knife_m9_bayonet";
    case weapon_knife_tactical: return "knife_tactical";
    case weapon_knife_falchion: return "knife_falchion";
    case weapon_knife_survival_bowie: return "knife_survival_bowie";
    case weapon_knife_butterfly: return "knife_butterfly";
    case weapon_knife_push: return "knife_push";
    case weapon_knife_cord: return "knife_cord";
    case weapon_knife_canis: return "knife_canis";
    case weapon_knife_ursus: return "knife_ursus";
    case weapon_knife_gypsy_jackknife: return "knife_gypsy_jackknife";
    case weapon_knife_outdoor: return "knife_outdoor";
    case weapon_knife_stiletto: return "knife_stiletto";
    case weapon_knife_widowmaker: return "knife_widowmaker";
    case weapon_knife_skeleton: return "knife_skeleton";
    case weapon_knife_kukri: return "knife_kukri";
    default: return "unknown";
    }
}