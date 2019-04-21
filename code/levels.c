#include "player.hpp"
#include "world.hpp"

PlatformInfo platform_info_0[] = {
    {Vec2(4.f,  4.5f), Vec2(0.f, +5.5f)},
    {Vec2(6.f,  4.5f), Vec2(0.f, +5.0f)},
    {Vec2(8.f,  4.5f), Vec2(0.f, +4.5f)},
    {Vec2(10.f, 4.5f), Vec2(0.f, +4.0f)},
    {Vec2(12.f, 4.5f), Vec2(0.f, +3.5f)},
    {Vec2(14.f, 4.5f), Vec2(0.f, +3.0f)},
};

PlatformInfo platform_info_1[] = {
    {Vec2(4.f,  2.5f), Vec2(+2.f, 0.f)},
    {Vec2(15.f, 2.5f), Vec2(+2.f, 0.f)},
    {Vec2(15.f, 8.5f), Vec2(+2.f, 0.f)},
    {Vec2(18.f, 4.5f), Vec2(0.f, +2.f)},
};

Vec2 enemy_info_0[] = {
    { 3.5f, 1.5f},
    { 7.5f, 1.5f},
    {11.5f, 1.5f},
    {15.5f, 1.5f},
};
Vec2 enemy_info_1[] = {
    { 1.5f, 1.5f},
    { 5.5f, 1.5f},
    { 9.5f, 1.5f},
    {13.5f, 1.5f},
};

#define PLATFORM_INFO(n) { ArraySize(platform_info_##n ), ArraySize(platform_info_##n ), platform_info_##n }
#define ENEMY_INFO(n) { ArraySize(enemy_info_##n ), ArraySize(enemy_info_##n ), enemy_info_##n }

LevelInfo all_levels[] = {
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(17, 10),
        // Start
        Vec2(3.5f, 2.5f), Vec2(3.5f, 2.5f),
        // Layout
        {
            "                 ",
            "                 ",
            "                 ",
            "                 ",
            "                 ",
            "                 ",
            "                 ",
            "  @    *   @#F#@ ",
            " @@@@      @#F#@@",
            "@@@@@@@@@@@@#F#@@",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(17, 10),
        // Start
        Vec2(3.5f, 2.5f), Vec2(3.5f, 9.5f),
        // Layout
        {
            "###j#############",
            "#DD.DvvvDDDDDDDD#",
            "#.....  ....  ..#",
            "#. .......... ..#",
            "#.........   +..#",
            "#..  ..  .......#",
            "#..  ...  ... ..#",
            "#...  ........  #",
            "#U###^UUUUUU#H#U#",
            "#############H###",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(17, 7),
        // Start
        Vec2(2.5f, 1.5f), Vec2(2.5f, 6.5f), 
        // Layout
        {
            "##j##############",
            "#1.v#vDv#vDvDvD7#",
            "#>. # . D . . .<#",
            "#>. D . . . U .<#",
            "#>. . . U . # +<#",
            "#>. U . # . # .<#",
            "##############H##",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(15, 12),
        // Start
        Vec2(12.5f, 1.5f), Vec2(12.5f, 11.5f), 
        // Layout
        {
            "############j##",
            "#1DvDvDvDvDv.7#",
            "#R . . . + . L#",
            "#>. . . . . .<#",
            "#R . . . . . L#",
            "#>. . . . . .<#",
            "#R . . # . . L#",
            "#>. . . . . .<#",
            "#R#. . . . . L#",
            "#>. . . . . .<#",
            "#/^U^U##^U^U .G",
            "###############",
        },
        // Exit side
        SIDE_RIGHT,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(9, 15),
        // Start
        Vec2(2.5f, 1.5f), Vec2(0.5f, 1.5f), 
        // Layout
        {
            "#########",
            "#   .   #",
            "# . . . #",
            "#   #   #",
            "#...#...#",
            "#   #   #",
            "# . # . #",
            "#   #   #",
            "#...#...#",
            "#   #   #",
            "# . # . #",
            "#   #   #",
            "#...#.+.#",
            "J   #^^^#",
            "##H######",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(9, 15),
        // Start
        Vec2(4.5f, 1.5f), Vec2(0.5f, 14.5f), 
        // Layout
        {
            "j########",
            "j#.....##",
            "j#..* .##",
            "j#..  .##",
            "j#.  ..##",
            "j#.  ..##",
            "j#    L##",
            "j#.   .##",
            "j#R.   ##",
            "j#..  .##",
            "j#.   <##",
            "j#..  .##",
            "j#R. ..##",
            "JJ.....##",
            "######H##",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(23, 13),
        // Start
        Vec2(20.5f, 10.5f), Vec2(20.5f, 12.5f), 
        // Layout
        {
            "####################j##",
            "#              ....   #",
            "#              .  .   #",
            "#              .<######",
            "#              ..#1vv7#",
            "#                #>  <#",
            "#                #>  <#",
            "#  +.............#>  <#",
            "#  .             #>  <#",
            "#  .             #>  <#",
            "#  .             #>  <#",
            "#^ . ^^^^^^^^^^^^#/^^]#",
            "###H###################",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(20, 16),
        // Start
        Vec2(3.5f, 11.5f), Vec2(3.5f, 15.5f), 
        // Layout
        {
            "###j################",
            "#.    .   ....   .L#",
            "#.    ..    .    .L#",
            "#    .....     ...<#",
            "#   ..  ....  ... <#",
            "###########U^^.   ]#",
            "##r###########R  L##",
            "#...  ...    .....L#",
            "#..   ..   ...    L#",
            "#..   ......    ..<#",
            "#.    ..     .  ..L#",
            "# *  ..   ..      L#",
            "#    ..  ...   aa.L#",
            "G.  ..    ..    ..L#",
            "#######^^^UUU^^UUU]#",
            "####################",
        },
        // Exit side
        SIDE_LEFT,
    },
    {
        // Enemies
        {0},
        // Platforms
        {0},
        // Size
        Vec2i(11, 16),
        // Start
        Vec2(5.5f, 1.5f), Vec2(10.5f, 1.5f), 
        // Layout
        {
            "###########",
            "#  . * .  #",
            "#  .   .  #",
            "#  .....  #",
            "#    .    #",
            "# .. ..bb.#",
            "#    .    #",
            "#.aa.. .. #",
            "#    .    #",
            "# .. ..bb.#",
            "#    .    #",
            "#.aa.. .. #",
            "#    .    #",
            "# .. ..bb.#",
            "# ^^ . ^^ J",
            "#####H#####",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        {0},
        // Platforms
        PLATFORM_INFO(0),
        // Size
        Vec2i(18, 9),
        // Start
        Vec2(1.5f, 1.5f), Vec2(0.5f, 1.5f),
        // Layout
        {
            "##################",
            "#..  ..  ..  ..  #",
            "G..  ..  ..  ..  #",
            "###  ..  ..  ..  #",
            "#..  ..  ..  .. *#",
            "#..  ..  ..  ..  #",
            "#..  ..  ..  ..  #",
            "J..XXZZXXZZXXZZ  #",
            "##################",
        },
        // Exit side
        SIDE_LEFT,
    },
    {
        // Enemies
        {0},
        // Platforms
        PLATFORM_INFO(1),
        // Size
        Vec2i(21, 14),
        // Start
        Vec2(1.5f, 3.5f), Vec2(0.5f, 3.5f), 
        // Layout
        {
            "#####################",
            "#1vvvvvvvvvvvvvvvvvv#",
            "#>                  #",
            "#>                  #",
            "#R........U.........#",
            "#> -      v      --.#",
            "#R+..............  .#",
            "#>              .  .#",
            "#>              .  .#",
            "#>              .  .#",
            "J................  .#",
            "#H#-      -      --.#",
            "#H#UUUUUUUUUUUUUUUUU#",
            "#H###################",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        ENEMY_INFO(0),
        // Platforms
        {0},
        // Size
        Vec2i(17, 6),
        // Start
        Vec2(14.5f, 2.5f), Vec2(14.5f, 5.5f), 
        // Layout
        {
            "##############j##",
            "#vvvvvvvvvvvvv v#",
            "#+..............#",
            "#               #",
            "#     ^       # #",
            "##H##############",
        },
        // Exit side
        SIDE_DOWN,
    },
    {
        // Enemies
        ENEMY_INFO(1),
        // Platforms
        {0},
        // Size
        Vec2i(17, 6),
        // Start
        Vec2(2.5f, 2.5f), Vec2(2.5f, 5.5f), 
        // Layout
        {
            "##j##############",
            "#v vvvvvvvvvvvvv#",
            "#..............+#",
            "#               #",
            "# #             #",
            "##############H##",
        },
        // Exit side
        SIDE_DOWN,
    },
};