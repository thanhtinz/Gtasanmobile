/**
 * Job framework.
 *
 * Modelled on GTA V's freelance work rather than SA-MP's usual "one class per
 * job": a player takes a job, clocks in, and the framework drives a chain of
 * destinations, paying per leg. Adding a job means adding a row to g_Jobs plus
 * choosing a destination pool — no new plumbing.
 *
 * Rank is per job, held in the job_progress table, so changing trade does not
 * throw away what you built up in the last one.
 */

#if defined _gtasan_jobs_included
	#endinput
#endif
#define _gtasan_jobs_included

#define JOB_NONE                (-1)
#define JOB_CHECKPOINT_SIZE     (5.0)
#define JOB_MAX_LEGS            (6)
#define JOB_RANK_MAX            (10)

// Categories, in the order they appear in the menu.
#define JOBCAT_DRIVING          (0)
#define JOBCAT_EXTRACTION       (1)
#define JOBCAT_FACTION          (2)
#define JOBCAT_ILLEGAL          (3)
#define JOBCAT_COUNT            (4)

// Destination pools. Keeping them separate is what makes a trucking run feel
// different from a taxi fare without duplicating any of the run logic.
#define POOL_CITY               (0)
#define POOL_FREIGHT            (1)
#define POOL_RURAL              (2)
#define POOL_AIR                (3)
#define POOL_SEA                (4)
#define POOL_CRIME              (5)

enum E_JOB
{
	jName[26],
	jSummary[92],
	jCategory,
	jMinLevel,
	jPayMin,
	jPayMax,
	jExp,
	jSkin,
	jVehicle,               // model handed out on clock-in, 0 for none
	jWanted,                // wanted stars added per completed leg
	Float:jX,               // where the job is signed up for
	Float:jY,
	Float:jZ,
	jPool
}

static const g_Jobs[][E_JOB] =
{
	// --- driving and transport ---------------------------------------------
	{"Taxi",            "Don khach quanh Los Santos, tra theo quang duong.",       JOBCAT_DRIVING,    1,  450,  900,  14, 253, 420, 0, 1735.0, -1861.0, 13.5, POOL_CITY},
	{"Xe bus",          "Chay tuyen bus co dinh trong thanh pho.",                 JOBCAT_DRIVING,    2,  500,  950,  15, 253, 437, 0, 1176.0, -1806.0, 16.5, POOL_CITY},
	{"Xe tai",          "Cho hang giua cac kho, tuyen dai va tra cao.",            JOBCAT_DRIVING,    3,  900, 1700,  22, 171, 515, 0, 2172.0, -2263.0, 13.3, POOL_FREIGHT},
	{"Xe rac",          "Thu gom rac, chang ngan nhung lien tuc.",                 JOBCAT_DRIVING,    1,  380,  700,  12, 261, 408, 0, 2179.0, -1969.0, 13.3, POOL_CITY},
	{"Chuyen phat",     "Giao buu kien gap khap Los Santos.",                      JOBCAT_DRIVING,    1,  420,  820,  13, 260, 448, 0, 1481.0, -1755.0, 13.5, POOL_CITY},
	{"Xe cuu thuong",   "Dua nguoi bi thuong ve benh vien.",                       JOBCAT_DRIVING,    6, 1000, 1800,  24, 275, 416, 0, 1172.0, -1323.0, 15.4, POOL_CITY},
	{"Phi cong",        "Bay hang giua cac san bay cua bang.",                     JOBCAT_DRIVING,   12, 2200, 3800,  40,  61, 519, 0, 1957.0, -2178.0, 13.5, POOL_AIR},
	{"Tau thuy",        "Cho hang doc bo bien San Andreas.",                       JOBCAT_DRIVING,   10, 1600, 2700,  32,  29, 453, 0, 2135.0, -2593.0,  0.5, POOL_SEA},

	// --- extraction and production -----------------------------------------
	{"Tho mo",          "Khai thac da o Bone County, nang nhung dang tien.",       JOBCAT_EXTRACTION, 6, 1100, 1900,  26, 260, 486, 0,  595.0,   862.0, -42.0, POOL_RURAL},
	{"Tho go",          "Don go trong rung Whetstone.",                            JOBCAT_EXTRACTION, 5,  950, 1600,  22, 161, 456, 0, -570.0, -1140.0,  75.0, POOL_RURAL},
	{"Ngu dan",         "Ra khoi danh ca roi mang ve cang ban.",                   JOBCAT_EXTRACTION, 3,  700, 1300,  18,  27, 453, 0, 2135.0, -2593.0,   0.5, POOL_SEA},
	{"Nong dan",        "Cho nong san giua cac trang trai Red County.",            JOBCAT_EXTRACTION, 4,  700, 1300,  18, 161, 532, 0, -370.0, -1428.0,  25.7, POOL_RURAL},
	{"Tho san",         "San thu trong rung, hang ban duoc gia.",                  JOBCAT_EXTRACTION, 8, 1200, 2000,  28, 162, 468, 0, -320.0,   -49.0,  63.2, POOL_RURAL},
	{"Tho sua xe",      "Di den cho xe hong va sua tai cho.",                      JOBCAT_EXTRACTION, 5,  800, 1500,  20,  50, 525, 0, 2064.0, -1831.0,  13.5, POOL_CITY},
	{"Cong nhan dau",   "Van hanh gian khoan va cho dau ve nha may.",              JOBCAT_EXTRACTION, 9, 1400, 2300,  30,  16, 514, 0,  -70.0,  -1170.0,   2.0, POOL_FREIGHT},

	// --- factions ----------------------------------------------------------
	{"Canh sat",        "Tuan tra, truy bat nguoi bi truy na.",                    JOBCAT_FACTION,   10, 1200, 2100,  30, 280, 596, 0, 1552.0, -1675.0,  16.2, POOL_CITY},
	{"Bac si",          "Cap cuu va hoi phuc cho nguoi choi khac.",                JOBCAT_FACTION,    8, 1100, 1900,  28, 275, 416, 0, 1172.0, -1323.0,  15.4, POOL_CITY},
	{"Cuu hoa",         "Toi hien truong chay va dap lua.",                        JOBCAT_FACTION,    8, 1000, 1700,  26, 277, 407, 0, 1777.0, -1310.0,  18.8, POOL_CITY},
	{"Ve si",           "Bao ve khach VIP tren duong di.",                         JOBCAT_FACTION,   12, 1500, 2400,  32, 163, 579, 0, 1481.0, -1755.0,  13.5, POOL_CITY},
	{"Cuu ho giao thong", "Keo xe hong ve bai.",                                   JOBCAT_FACTION,    4,  750, 1400,  19,  50, 525, 0, 2064.0, -1831.0,  13.5, POOL_CITY},

	// --- illegal -----------------------------------------------------------
	{"Trom xe",         "Nhan don trom xe roi giao cho bai thao.",                 JOBCAT_ILLEGAL,   14, 2000, 3400,  38, 137, 0,   1, 2755.0,  -1980.0,  13.5, POOL_CRIME},
	{"Cuop cua hang",   "Danh nhanh cac cua hang tien loi.",                       JOBCAT_ILLEGAL,   16, 2400, 4000,  42, 137, 0,   2, 2421.0,  -1509.0,  24.0, POOL_CRIME},
	{"Buon hang cam",   "Cho hang cam qua mat canh sat.",                          JOBCAT_ILLEGAL,   18, 3000, 5200,  48, 137, 482, 2, -1620.0,   120.0,   3.5, POOL_FREIGHT},
	{"Rua tien",        "Chuyen tien ban qua cac diem trung gian.",                JOBCAT_ILLEGAL,   20, 3400, 5800,  50, 137, 0,   1, 2004.0,  -1114.0,  27.0, POOL_CITY},
	{"Cuop ngan hang",  "Phi vu lon, can dong doi va rat nhieu gan.",              JOBCAT_ILLEGAL,   25, 6000,10000,  80, 137, 0,   4, 1462.0,  -1012.0,  26.8, POOL_CRIME}
};

#define JOB_COUNT               (sizeof (g_Jobs))

static const g_CategoryNames[JOBCAT_COUNT][24] =
{
	"Lai xe / van chuyen",
	"Khai thac / san xuat",
	"Phe phai",
	"Phi phap"
};

// --- destination pools ------------------------------------------------------
static const Float:g_PoolCity[][3] =
{
	{1481.0, -1755.0, 13.5}, {2064.0, -1831.0, 13.5}, {1176.0, -1806.0, 16.5},
	{2179.0, -1969.0, 13.3}, {1735.0, -1861.0, 13.5}, {2495.0, -1687.0, 13.5},
	{1358.0, -1278.0, 13.5}, { 647.0, -1372.0, 13.5}, {1029.0, -1339.0, 13.5},
	{2233.0, -1159.0, 25.9}, {1833.0, -1130.0, 23.9}, {1367.0, -1683.0, 13.5},
	{2118.0, -2270.0, 13.5}, {2749.0, -2450.0, 13.6}, {2807.0, -1174.0, 25.0}
};

static const Float:g_PoolFreight[][3] =
{
	{2172.0, -2263.0, 13.3}, {2749.0, -2450.0, 13.6}, {2118.0, -2270.0, 13.5},
	{-1622.0,  118.0,  3.5}, {-2224.0,  245.0, 35.3}, {1937.0, -2110.0, 13.5},
	{-1043.0, -650.0, 32.0}, { 2841.0,  927.0, 10.7}, {1613.0,  1799.0, 10.8}
};

static const Float:g_PoolRural[][3] =
{
	{ 595.0,   862.0, -42.0}, {-370.0, -1428.0, 25.7}, {-1067.0, -1226.0, 129.0},
	{-320.0,   -49.0,  63.2}, { 620.0,  1210.0, 11.5}, {-1108.0, -1620.0,  76.4},
	{ 135.0,  1160.0,  15.8}, {-547.0,  -520.0, 25.5}, { -570.0, -1140.0,  75.0}
};

static const Float:g_PoolAir[][3] =
{
	{1957.0, -2178.0, 13.5}, {-1236.0, -20.0, 14.1}, {1657.0, 1601.0, 10.8},
	{ 394.0,  2537.0, 16.5}, {-1531.0, -441.0,  6.0}
};

static const Float:g_PoolSea[][3] =
{
	{2135.0, -2593.0,  0.5}, {-1610.0,  1207.0, 0.5}, { 720.0, -1975.0, 0.5},
	{-2440.0,  1520.0, 0.5}, {  380.0, -2080.0, 0.5}
};

static const Float:g_PoolCrime[][3] =
{
	{2755.0, -1980.0, 13.5}, {2421.0, -1509.0, 24.0}, {2004.0, -1114.0, 27.0},
	{1462.0, -1012.0, 26.8}, { 661.0, -1275.0, 13.5}, {2352.0,   -20.0, 26.5},
	{-2119.0,  -424.0, 35.5}, {1930.0,  2308.0, 10.8}
};

// --- per-player state -------------------------------------------------------
enum E_SHIFT
{
	bool:sWorking,
	sLeg,                   // legs completed in the current run
	sLegs,                  // legs this run is made of
	sVehicle,
	Float:sTargetX,
	Float:sTargetY,
	Float:sTargetZ,
	Float:sStartX,
	Float:sStartY,
	Float:sStartZ,
	sEarned
}

static Shift[MAX_PLAYERS][E_SHIFT];

// Rank and experience per job, mirrored from the job_progress table.
static
	JobRank[MAX_PLAYERS][JOB_COUNT],
	JobExp[MAX_PLAYERS][JOB_COUNT],
	JobRuns[MAX_PLAYERS][JOB_COUNT],
	bool:JobDirty[MAX_PLAYERS][JOB_COUNT];

forward OnJobProgressLoaded(playerid, extra);

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

stock bool:Job_IsValid(jobid)
{
	return 0 <= jobid < JOB_COUNT;
}

stock Job_Name(jobid, dest[], size = sizeof (dest))
{
	if (!Job_IsValid(jobid))
	{
		format(dest, size, "Khong co");
	}
	else
	{
		format(dest, size, "%s", g_Jobs[jobid][jName]);
	}
	return 1;
}

stock Job_CategoryName(category, dest[], size = sizeof (dest))
{
	if (!(0 <= category < JOBCAT_COUNT))
	{
		format(dest, size, "?");
	}
	else
	{
		format(dest, size, "%s", g_CategoryNames[category]);
	}
	return 1;
}

stock Job_Count() { return JOB_COUNT; }
stock Job_MinLevel(jobid) { return Job_IsValid(jobid) ? g_Jobs[jobid][jMinLevel] : 0; }
stock Job_Category(jobid) { return Job_IsValid(jobid) ? g_Jobs[jobid][jCategory] : -1; }
stock Job_PayMin(jobid) { return Job_IsValid(jobid) ? g_Jobs[jobid][jPayMin] : 0; }
stock Job_PayMax(jobid) { return Job_IsValid(jobid) ? g_Jobs[jobid][jPayMax] : 0; }
stock bool:Job_IsWorking(playerid) { return Shift[playerid][sWorking]; }

stock Job_Rank(playerid, jobid)
{
	return Job_IsValid(jobid) ? JobRank[playerid][jobid] : 0;
}

/// Pay multiplier from rank: rank 0 pays face value, rank 10 pays double.
stock Float:Job_RankMultiplier(playerid, jobid)
{
	new rank = Job_Rank(playerid, jobid);
	if (rank < 0)
	{
		rank = 0;
	}
	else if (rank > JOB_RANK_MAX)
	{
		rank = JOB_RANK_MAX;
	}
	return 1.0 + (float(rank) / 10.0);
}

stock Job_RequiredExp(rank)
{
	return 300 + (rank * 450);
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

stock Job_ResetPlayer(playerid)
{
	new emptyShift[E_SHIFT];
	Shift[playerid] = emptyShift;

	for (new i = 0; i < JOB_COUNT; ++i)
	{
		JobRank[playerid][i]  = 0;
		JobExp[playerid][i]   = 0;
		JobRuns[playerid][i]  = 0;
		JobDirty[playerid][i] = false;
	}
	return 1;
}

stock Job_LoadProgress(playerid)
{
	if (Player[playerid][pAccountID] <= 0)
	{
		return 0;
	}

	new query[160];
	format(query, sizeof query,
		"SELECT job_id, rank, exp, runs FROM job_progress WHERE account_id = %d;",
		Player[playerid][pAccountID]);
	DB_QueryCB(query, "OnJobProgressLoaded", playerid);
	return 1;
}

public OnJobProgressLoaded(playerid, extra)
{
	#pragma unused extra

	if (!IsPlayerConnectedEx(playerid))
	{
		return 0;
	}

	while (DB_Next())
	{
		new jobid = DB_Int("job_id");
		// A row for a job that no longer exists is skipped rather than trusted;
		// job ids shift when the table above is reordered.
		if (!Job_IsValid(jobid))
		{
			continue;
		}

		JobRank[playerid][jobid] = DB_Int("rank");
		JobExp[playerid][jobid]  = DB_Int("exp");
		JobRuns[playerid][jobid] = DB_Int("runs");
	}
	return 1;
}

/// Writes back only the jobs touched this session.
stock Job_SaveProgress(playerid)
{
	if (Player[playerid][pAccountID] <= 0)
	{
		return 0;
	}

	new query[320];
	for (new jobid = 0; jobid < JOB_COUNT; ++jobid)
	{
		if (!JobDirty[playerid][jobid])
		{
			continue;
		}

		// Both engines accept this upsert form, and the unique index on
		// (account_id, job_id) is what makes it collapse to an update.
		format(query, sizeof query,
			"INSERT INTO job_progress (account_id, job_id, rank, exp, runs) "\
			"VALUES (%d, %d, %d, %d, %d) "\
			SQL_UPSERT("account_id, job_id")\
			"rank = %d, exp = %d, runs = %d;",
			Player[playerid][pAccountID], jobid,
			JobRank[playerid][jobid], JobExp[playerid][jobid], JobRuns[playerid][jobid],
			JobRank[playerid][jobid], JobExp[playerid][jobid], JobRuns[playerid][jobid]);
		DB_Query(query);

		JobDirty[playerid][jobid] = false;
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Joining and leaving
// ---------------------------------------------------------------------------

stock Job_Join(playerid, jobid)
{
	if (!Job_IsValid(jobid))
	{
		return 0;
	}

	if (Player[playerid][pLevel] < g_Jobs[jobid][jMinLevel])
	{
		SendClientMessage(playerid, COLOR_ERROR,
			"Nghe nay yeu cau level %d, ban dang level %d.",
			g_Jobs[jobid][jMinLevel], Player[playerid][pLevel]);
		return 0;
	}

	if (Player[playerid][pJob] == jobid)
	{
		SendClientMessage(playerid, COLOR_GREY, "Ban dang lam nghe nay roi.");
		return 0;
	}

	Job_EndShift(playerid, false);
	Player[playerid][pJob] = jobid;

	SendClientMessage(playerid, COLOR_JOB, "Ban da nhan nghe %s (bac %d).",
		g_Jobs[jobid][jName], JobRank[playerid][jobid]);
	SendClientMessage(playerid, COLOR_GREY, "%s", g_Jobs[jobid][jSummary]);
	SendClientMessage(playerid, COLOR_GREY, "Den diem lam viec da danh dau tren ban do roi bam Vao ca.");

	Job_MarkWorkpoint(playerid);
	return 1;
}

stock Job_Leave(playerid, bool:notify)
{
	if (Player[playerid][pJob] == JOB_NONE)
	{
		if (notify)
		{
			SendClientMessage(playerid, COLOR_GREY, "Ban dang khong lam nghe nao.");
		}
		return 0;
	}

	Job_EndShift(playerid, false);

	if (notify)
	{
		SendClientMessage(playerid, COLOR_JOB, "Ban da nghi nghe %s. Bac nghe van duoc giu lai.",
			g_Jobs[Player[playerid][pJob]][jName]);
	}

	Player[playerid][pJob] = JOB_NONE;
	RemovePlayerMapIcon(playerid, 1);
	return 1;
}

/// Marks the sign-up point of the current job on the map.
stock Job_MarkWorkpoint(playerid)
{
	new jobid = Player[playerid][pJob];
	if (!Job_IsValid(jobid))
	{
		return 0;
	}
	SetPlayerMapIcon(playerid, 1, g_Jobs[jobid][jX], g_Jobs[jobid][jY], g_Jobs[jobid][jZ],
		GM_MAP_ICON_JOB, 0, MAPICON_LOCAL);
	return 1;
}

// ---------------------------------------------------------------------------
// Shifts
// ---------------------------------------------------------------------------

stock Job_StartShift(playerid)
{
	new jobid = Player[playerid][pJob];

	if (!Job_IsValid(jobid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban chua co nghe. Mo /menu de chon.");
		return 0;
	}

	if (Shift[playerid][sWorking])
	{
		SendClientMessage(playerid, COLOR_GREY, "Ban dang trong ca lam roi.");
		return 0;
	}

	if (!IsPlayerInRangeOfPoint(playerid, 12.0, g_Jobs[jobid][jX], g_Jobs[jobid][jY], g_Jobs[jobid][jZ]))
	{
		SendClientMessage(playerid, COLOR_ERROR,
			"Ban phai o diem lam viec cua nghe %s (da danh dau tren ban do).",
			g_Jobs[jobid][jName]);
		return 0;
	}

	Shift[playerid][sWorking] = true;
	Shift[playerid][sLeg]     = 0;
	Shift[playerid][sLegs]    = RandomEx(2, JOB_MAX_LEGS + 1);
	Shift[playerid][sEarned]  = 0;

	if (g_Jobs[jobid][jSkin] > 0)
	{
		SetPlayerSkin(playerid, g_Jobs[jobid][jSkin]);
	}

	if (g_Jobs[jobid][jVehicle] > 0)
	{
		new Float:x, Float:y, Float:z, Float:angle;
		GetPlayerPos(playerid, x, y, z);
		GetPlayerFacingAngle(playerid, angle);

		Shift[playerid][sVehicle] = CreateVehicle(g_Jobs[jobid][jVehicle],
			x + 4.0, y + 4.0, z + 0.5, angle, -1, -1, 600, false);
		PutPlayerInVehicle(playerid, Shift[playerid][sVehicle], 0);
	}

	SendClientMessage(playerid, COLOR_JOB, "Bat dau ca lam: %d chang.", Shift[playerid][sLegs]);
	Job_NextLeg(playerid);
	return 1;
}

stock Job_EndShift(playerid, bool:notify)
{
	if (!Shift[playerid][sWorking])
	{
		return 0;
	}

	DisablePlayerCheckpoint(playerid);

	if (Shift[playerid][sVehicle] > 0)
	{
		DestroyVehicle(Shift[playerid][sVehicle]);
	}

	if (notify)
	{
		SendClientMessage(playerid, COLOR_JOB, "Ket thuc ca lam. Tong thu nhap: %s.",
			Money(Shift[playerid][sEarned]));
	}

	new empty[E_SHIFT];
	Shift[playerid] = empty;
	return 1;
}

static Job_PickDestination(pool, &Float:x, &Float:y, &Float:z)
{
	switch (pool)
	{
		case POOL_FREIGHT:
		{
			new index = random(sizeof (g_PoolFreight));
			x = g_PoolFreight[index][0]; y = g_PoolFreight[index][1]; z = g_PoolFreight[index][2];
		}
		case POOL_RURAL:
		{
			new index = random(sizeof (g_PoolRural));
			x = g_PoolRural[index][0]; y = g_PoolRural[index][1]; z = g_PoolRural[index][2];
		}
		case POOL_AIR:
		{
			new index = random(sizeof (g_PoolAir));
			x = g_PoolAir[index][0]; y = g_PoolAir[index][1]; z = g_PoolAir[index][2];
		}
		case POOL_SEA:
		{
			new index = random(sizeof (g_PoolSea));
			x = g_PoolSea[index][0]; y = g_PoolSea[index][1]; z = g_PoolSea[index][2];
		}
		case POOL_CRIME:
		{
			new index = random(sizeof (g_PoolCrime));
			x = g_PoolCrime[index][0]; y = g_PoolCrime[index][1]; z = g_PoolCrime[index][2];
		}
		default:
		{
			new index = random(sizeof (g_PoolCity));
			x = g_PoolCity[index][0]; y = g_PoolCity[index][1]; z = g_PoolCity[index][2];
		}
	}
	return 1;
}

stock Job_NextLeg(playerid)
{
	new jobid = Player[playerid][pJob];
	if (!Job_IsValid(jobid) || !Shift[playerid][sWorking])
	{
		return 0;
	}

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	Shift[playerid][sStartX] = x;
	Shift[playerid][sStartY] = y;
	Shift[playerid][sStartZ] = z;

	// Re-roll a destination that landed on top of the player; a zero-length leg
	// would pay for nothing.
	new Float:tx, Float:ty, Float:tz;
	for (new attempt = 0; attempt < 8; ++attempt)
	{
		Job_PickDestination(g_Jobs[jobid][jPool], tx, ty, tz);
		if (VectorSize(tx - x, ty - y, tz - z) > 150.0)
		{
			break;
		}
	}

	Shift[playerid][sTargetX] = tx;
	Shift[playerid][sTargetY] = ty;
	Shift[playerid][sTargetZ] = tz;

	SetPlayerCheckpoint(playerid, tx, ty, tz, JOB_CHECKPOINT_SIZE);

	new zone[40];
	Zone_Name(tx, ty, zone, sizeof zone);
	SendClientMessage(playerid, COLOR_JOB, "Chang %d/%d - di den %s (da danh dau).",
		Shift[playerid][sLeg] + 1, Shift[playerid][sLegs], zone);
	return 1;
}

/// Called from OnPlayerEnterCheckpoint. Returns 1 when the checkpoint was ours.
stock Job_OnCheckpoint(playerid)
{
	if (!Shift[playerid][sWorking])
	{
		return 0;
	}

	new jobid = Player[playerid][pJob];
	if (!Job_IsValid(jobid))
	{
		return 0;
	}

	if (!IsPlayerInRangeOfPoint(playerid, JOB_CHECKPOINT_SIZE + 3.0,
		Shift[playerid][sTargetX], Shift[playerid][sTargetY], Shift[playerid][sTargetZ]))
	{
		return 0;
	}

	// Pay scales with how far the leg actually was, so a lucky short hop does
	// not pay the same as a cross-state haul.
	new Float:distance = VectorSize(
		Shift[playerid][sTargetX] - Shift[playerid][sStartX],
		Shift[playerid][sTargetY] - Shift[playerid][sStartY],
		Shift[playerid][sTargetZ] - Shift[playerid][sStartZ]);

	new base = RandomEx(g_Jobs[jobid][jPayMin], g_Jobs[jobid][jPayMax] + 1);
	new distanceBonus = floatround(distance / 6.0);
	new pay = floatround(float(base + distanceBonus) * Job_RankMultiplier(playerid, jobid));

	Money_Give(playerid, pay, "job payout");
	Shift[playerid][sEarned] += pay;
	Level_GiveExp(playerid, g_Jobs[jobid][jExp], g_Jobs[jobid][jName]);
	Job_GiveJobExp(playerid, jobid, g_Jobs[jobid][jExp]);

	// Illegal work is paid better because it puts the police on you.
	if (g_Jobs[jobid][jWanted] > 0)
	{
		Wanted_Add(playerid, g_Jobs[jobid][jWanted]);
	}

	DisablePlayerCheckpoint(playerid);
	Shift[playerid][sLeg]++;

	SendClientMessage(playerid, COLOR_MONEY, "Hoan thanh chang: +%s (%d m).",
		Money(pay), floatround(distance));

	if (Shift[playerid][sLeg] >= Shift[playerid][sLegs])
	{
		new bonus = Shift[playerid][sEarned] / 5;
		Money_Give(playerid, bonus, "shift bonus");
		Shift[playerid][sEarned] += bonus;
		JobRuns[playerid][jobid]++;
		JobDirty[playerid][jobid] = true;

		SendClientMessage(playerid, COLOR_SUCCESS, "Hoan thanh ca lam! Thuong them %s.", Money(bonus));
		Job_EndShift(playerid, true);
	}
	else
	{
		Job_NextLeg(playerid);
	}
	return 1;
}

stock Job_GiveJobExp(playerid, jobid, amount)
{
	if (!Job_IsValid(jobid))
	{
		return 0;
	}

	JobExp[playerid][jobid] += amount;
	JobDirty[playerid][jobid] = true;

	while (JobRank[playerid][jobid] < JOB_RANK_MAX)
	{
		new required = Job_RequiredExp(JobRank[playerid][jobid]);
		if (JobExp[playerid][jobid] < required)
		{
			break;
		}

		JobExp[playerid][jobid] -= required;
		JobRank[playerid][jobid]++;

		SendClientMessage(playerid, COLOR_JOB,
			"Thang bac nghe %s: bac %d/%d - luong nhan %d%%.",
			g_Jobs[jobid][jName], JobRank[playerid][jobid], JOB_RANK_MAX,
			floatround(Job_RankMultiplier(playerid, jobid) * 100.0));
	}
	return 1;
}

/// Fills `dest` with a tab-separated dialog row for the job list.
stock Job_ListRow(playerid, jobid, dest[], size = sizeof (dest))
{
	new bool:unlocked = (Player[playerid][pLevel] >= g_Jobs[jobid][jMinLevel]);

	format(dest, size, "%s%s\t%s%d\t%sbac %d\t%s%d - %d",
		unlocked ? (EMBED_WHITE) : (EMBED_GREY), g_Jobs[jobid][jName],
		unlocked ? (EMBED_SUCCESS) : (EMBED_ERROR), g_Jobs[jobid][jMinLevel],
		EMBED_LEVEL, JobRank[playerid][jobid],
		EMBED_MONEY, g_Jobs[jobid][jPayMin], g_Jobs[jobid][jPayMax]);
	return 1;
}

stock Job_ShiftInfo(playerid, dest[], size = sizeof (dest))
{
	if (!Shift[playerid][sWorking])
	{
		format(dest, size, "Khong trong ca lam");
	}
	else
	{
		format(dest, size, "Chang %d/%d, da kiem %s",
			Shift[playerid][sLeg] + 1, Shift[playerid][sLegs], Money(Shift[playerid][sEarned]));
	}
	return 1;
}
