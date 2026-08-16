/**
 * San Andreas zone lookup.
 *
 * Used for "where am I" messages, job dispatch text and the police radio. This
 * is a coarse city/district split rather than the full 300-entry SA zone list:
 * it covers everywhere the gamemode actually sends players, and staying small
 * matters on the phone-class VPS these servers tend to run on.
 */

#if defined _gtasan_zones_included
	#endinput
#endif
#define _gtasan_zones_included

enum E_ZONE
{
	zName[40],
	Float:zMinX,
	Float:zMinY,
	Float:zMaxX,
	Float:zMaxY
}

static const g_Zones[][E_ZONE] =
{
	{"Los Santos",          44.6,   -2892.9,  2997.0,  -768.0},
	{"San Fierro",         -2997.0, -1115.5, -1213.9,   1075.0},
	{"Las Venturas",        869.4,   596.3,   2997.0,   2993.8},
	{"Red County",         -1213.9,  -768.0,  2997.0,    596.3},
	{"Flint County",       -1213.9, -2892.9,   44.6,    -768.0},
	{"Whetstone",          -2997.0, -2892.9, -1213.9,  -1115.5},
	{"Bone County",        -480.5,   596.3,   869.4,    2993.8},
	{"Tierra Robada",      -2997.0,  1075.0,  -480.5,   2993.8},
	{"Bayside",            -2741.0,  2175.1, -2270.0,   2660.4},
	{"Grove Street",        2380.9, -1751.5,  2582.0,  -1622.6},
	{"Idlewood",            1812.6, -1852.2,  2124.6,  -1602.3},
	{"San bay Los Santos",  1236.0, -2394.3,  2148.9,  -2078.9},
	{"Vinewood",            647.7,  -1372.0,  1276.6,  -1130.8},
	{"Trung tam LS",        1360.0, -1750.0,  1812.6,  -1152.0},
	{"The Strip",           2027.4,  1198.1,  2087.9,   1703.2},
	{"Old Venturas Strip",  2162.9,  2012.1,  2685.2,   2202.6},
	{"Doherty",            -2270.0, -1210.0, -1794.9,   -768.0},
	{"Easter Basin",       -1794.9, -1055.9, -1213.9,   -405.4}
};

stock Zone_Name(Float:x, Float:y, dest[], size = sizeof (dest))
{
	// Later entries are the more specific districts, so the scan runs backwards
	// and the first hit is the tightest match.
	for (new i = sizeof (g_Zones) - 1; i >= 0; --i)
	{
		if (g_Zones[i][zMinX] <= x <= g_Zones[i][zMaxX]
			&& g_Zones[i][zMinY] <= y <= g_Zones[i][zMaxY])
		{
			format(dest, size, "%s", g_Zones[i][zName]);
			return 1;
		}
	}

	format(dest, size, "San Andreas");
	return 0;
}

stock Zone_OfPlayer(playerid, dest[], size = sizeof (dest))
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	return Zone_Name(x, y, dest, size);
}

stock Zones_Init()
{
	Log("zones loaded (%d)", sizeof (g_Zones));
	return 1;
}
