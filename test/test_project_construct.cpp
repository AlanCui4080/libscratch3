#include <project.hpp>
#include <player.hpp>
int main()
{
    [[maybe_unused]] auto a = libsc3::player();
    [[maybe_unused]] auto p = libsc3::project("./basic_sb3.sb3");
    return 0;
}
