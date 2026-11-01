#include "tws_test_helpers.h"

extern "C" {
#include "formats.h"
#include "format-tws.h"
}

#include "data/ccl/ccl_embeds.h"
#include "data/tws/tws_embeds.h"

class GenericLogicTestMS : public testing::Test {
protected:
  LevelSet* level_set = nullptr;
  TWSSet* tws_set = nullptr;
  Level* level = nullptr;
  TWSMetadata* solution = nullptr;
  GameInputList input_list = {};

  void LoadLevel(char const pass[10]) {
    if (level != nullptr)
      Level_free(level);

    Result_LevelPtr res_level = LevelMetadata_make_level(LevelSet_get_level_by_pass(level_set, pass), &ms_logic);
    ASSERT_TRUE(res_level.success);
    level = res_level.value;
  }

  void LoadLevelTWS(char const pass[4]) {
    if (input_list.inputs != nullptr)
      GameInputList_free(&input_list);
    LoadLevel(pass);

    solution = TWSSet_get_solution_by_password(tws_set, level->metadata->password);
    ASSERT_NE(solution, nullptr);
    Result_GameInputList res3 = TWSMetadata_prepare_inputs(solution);
    ASSERT_TRUE(res3.success);
    input_list = res3.value;
  }

  void TickLevel(GameInput input) {
    Level_set_game_input(level, input);
    Level_tick(level);
    Level_set_game_input(level, INPUT_NIL);
    Level_tick(level);
  }

  void QuadTickLevel(GameInput input) {
    TickLevel(input);
    TickLevel(INPUT_NIL);
  }
};

class TrapDatLogicTestMS : public GenericLogicTestMS { // Put into its own since editing a trap DAT will require manual editing after
protected:
  TrapDatLogicTestMS() : GenericLogicTestMS() {
    Result_LevelSetPtr res = parse_ccl(TrapDatTestsMS_ccl, sizeof(TrapDatTestsMS_ccl));
    EXPECT_TRUE(res.success);
    level_set = res.value;
  }
};

class RegressionDatLogicTestMS : public GenericLogicTestMS {
protected:
  RegressionDatLogicTestMS() : GenericLogicTestMS() {
    Result_LevelSetPtr res = parse_ccl(RegressionTestsMS_ccl, sizeof(RegressionTestsMS_ccl));
    EXPECT_TRUE(res.success);
    level_set = res.value;

    Result_TWSSetPtr res2 = parse_tws(RegressionTests_ms_tws, sizeof(RegressionTests_ms_tws));
    EXPECT_TRUE(res2.success);
    tws_set = res2.value;
  }
};

namespace {
  TEST_F(TrapDatLogicTestMS, TrapOpenChip) {
    LoadLevel("TRPO");
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 0)), TileID_actor_with_dir(ACTOR_CHIP, DIRECTION_SOUTH));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(0, 0)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(1, 0)), TILE_EXIT);
    QuadTickLevel(DIRECTION_EAST);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 0)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(1, 0)), TileID_actor_with_dir(ACTOR_CHIP, DIRECTION_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(1, 0)), TILE_EXIT);
  }

  TEST_F(TrapDatLogicTestMS, TrapClosedChip) {
    LoadLevel("TRPC");
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 0)), TileID_actor_with_dir(ACTOR_CHIP, DIRECTION_SOUTH));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(0, 0)), TILE_TRAP);
    QuadTickLevel(DIRECTION_EAST);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 0)), TileID_actor_with_dir(ACTOR_CHIP, DIRECTION_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(0, 0)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(1, 0)), TILE_EXIT);
  }

  TEST_F(TrapDatLogicTestMS, TrapOpenGeneral) {
    LoadLevel("TRPO2");
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 1)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 1)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 1)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 1)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 1)), TILE_WATER);

    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 3)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(1, 3)), TILE_BUTTON_CLONE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 3)), TileID_actor_with_dir(ACTOR_BLOCK, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 3)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 3)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 3)), TILE_WATER);

    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 5)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TILE_FLOOR);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 6)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 5)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 5)), TILE_WATER);

    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_NIL);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 1)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 1)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 1)), TILE_DIRT);

    LoadLevel("TRPO2");
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_NIL);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 3)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 3)), TILE_DIRT);

    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    QuadTickLevel(INPUT_NORTH);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 4)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_NORTH));

    LoadLevel("TRPO2");
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_NORTH);
    QuadTickLevel(INPUT_NORTH);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_NORTH));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 4)), TILE_BLOCK_STATIC);
  }

  TEST_F(TrapDatLogicTestMS, TrapClosedGeneral) {
    LoadLevel("TRPC2");
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 1)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 1)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 1)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 1)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 1)), TILE_WATER);

    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 3)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(1, 3)), TILE_BUTTON_CLONE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 3)), TileID_actor_with_dir(ACTOR_BLOCK, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 3)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 3)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 3)), TILE_WATER);

    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(0, 5)), TILE_BUTTON_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TILE_FLOOR);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 6)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(3, 5)), TILE_ICE);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 5)), TILE_WATER);

    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_NIL);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 1)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 1)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 1)), TILE_WATER);

    LoadLevel("TRPC2");
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_NIL);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 3)), TileID_actor_with_dir(ACTOR_BLOCK, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 3)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 3)), TILE_WATER);
    QuadTickLevel(INPUT_WEST);
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_NIL);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 3)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(4, 3)), TILE_DIRT);

    LoadLevel("TRPC2");
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_EAST));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    QuadTickLevel(INPUT_NORTH);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TileID_actor_with_dir(ACTOR_CHIP, INPUT_NORTH));
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);

    LoadLevel("TRPC2");
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_SOUTH);
    QuadTickLevel(INPUT_EAST);
    QuadTickLevel(INPUT_NORTH);
    QuadTickLevel(INPUT_NORTH);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 5)), TILE_BLOCK_STATIC);
    EXPECT_EQ(Level_get_bottom_terrain(level, Position_from_xy(2, 5)), TILE_TRAP);
    EXPECT_EQ(Level_get_top_terrain(level, Position_from_xy(2, 4)), TILE_FLOOR);
  }

  TEST_F(RegressionDatLogicTestMS, MismatchedCloneIntoDirection) {
    LoadLevelTWS("YMEG");
    test_level(level, solution, &input_list);
  }
}
