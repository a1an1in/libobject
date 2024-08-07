/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Sdl_Image.h>
#include <libobject/ui/Sdl_Audio.h>
#include <libobject/event/event_compat.h>
#include <libobject/media/Player.h>
#include <libobject/media/FF_Extractor.h>
#include <libobject/media/FF_Video_Codec.h>
#include <libobject/media/FF_Audio_Codec.h>
#include <libobject/media/FF_Media_Source.h>
#include <libobject/media/FF_Window.h>
#include <libobject/media/FF_Event.h>
#include <libobject/media/message.h>
#include "Player_Command.h"

int player(int argc, char **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Player *player;
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    Audio *audio;
    Image *image;
    char *url;

    if (argc >= 1) {
        url = argv[0];
        dbg_str(DBG_DETAIL,"url : %s", url);
    } else {
        /*
         *url = "http://192.168.31.199/hls/test.m3u8";
         */
        url = "http://yunshangvideo.oss-cn-beijing.aliyuncs.com/mts_out/m3u8test/x36xhzz/x36xhzz.m3u8";
        /*
         *url = "http://192.168.199.136:8080/CloudSan/cloudsan.m3u8";
         */
        /*
         *url = "http://ys-qf.cloutropy.com/mts_in/softlink/17c32c42e3bd68c407cd99906c09a99e.mkv";
         */
        /*
         *url = "http://ys-mf.cloutropy.com/mts_in/softlink/0fa70594873d0a3701661c22883efdc8.mp4";
         */
    }
    c = cfg_alloc(allocator); 
    cfg_config_str(c, "/Window", "name", "Window");
    cfg_config_num(c, "/Window", "screen_width", 900);
    cfg_config_num(c, "/Window", "screen_height", 600);
    cfg_config_num(c, "/Window/Component/Container/Subject", "x", 0);
    cfg_config_num(c, "/Window/Component/Container/Subject", "y", 0);
    cfg_config_num(c, "/Window/Component/Container/Subject", "width", 900);
    cfg_config_num(c, "/Window/Component/Container/Subject", "height", 600);
    dbg_str(DBG_DETAIL, "config:%s", c->buf);

    FF_Window *window;
    Window *w;
    window = OBJECT_NEW(allocator, FF_Window, c->buf);
    w = (Window *)window;

    audio  = OBJECT_NEW(allocator, Sdl_Audio, NULL);

    image  = OBJECT_NEW(allocator, Sdl_Image, NULL);
    image->component.render = w->render;
    image->set_size(image, w->screen_width, w->screen_height);
    image->set_name(image, "player_image");

    player = OBJECT_NEW(allocator, Player, NULL);
    player->add_image(player, image);
    player->add_audio(player, audio);
    player->init(player);
    player->open(player, url);
    player->run(player);

    dbg_str(DBG_WARN, "render addr:%p", w->render);
    window->add_player(window, player);
    window->add_component((Container *)window, NULL, image);
    window->update_window((Window *)window);
    window->poll_event(window);

    object_destroy(audio);
    object_destroy(w);
    object_destroy(player);
    cfg_destroy(c);

    return 1;
}

static int __run_command(Player_Command *command)
{
    Command *c = (Command *)command;

    dbg_str(DBG_SUC, "player command");
    player(c->argc - 1, c->argv + 1);
    dbg_str(DBG_SUC, "player command end");

    return 1;
}
static int __construct(Player_Command *command, char *init_str)
{
    command->set(command, "/Command/name", "player");

    return 0;
}

static int __deconstruct(Player_Command *command)
{
    return 0;
}

static class_info_entry_t player_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Player_Command, construct, __construct),
    Init_Nfunc_Entry(2, Player_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Player_Command, set, NULL),
    Init_Vfunc_Entry(4, Player_Command, get, NULL),
    Init_Vfunc_Entry(5, Player_Command, run_command, __run_command),
    Init_End___Entry(6, Player_Command),
};
REGISTER_APP_CMD(Player_Command, player_command_class_info);
