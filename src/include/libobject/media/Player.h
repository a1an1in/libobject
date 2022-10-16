#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdio.h>
#include <signal.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/message/Publisher.h> 
#include <libobject/message/Centor.h>
#include <libobject/message/Subscriber.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/ui/Image.h>
#include <libobject/ui/Audio.h>
#include <libobject/core/Lock.h>
#include <libobject/media/MediaPeriod.h>
#include <libobject/media/Media_Source.h>
#include <libobject/media/DataSource.h>
#include <libobject/media/Extractor.h>
#include <libobject/media/PacketStream.h>
#include <libobject/media/FrameStream.h>
#include <libobject/media/Codec.h>
#include <libobject/media/AVSync.h>

typedef struct player_s Player;

#define MAX_AUDIO_PACKET_QUEUE_SIZE 500
#define MAX_VIDEO_PACKET_QUEUE_SIZE 500
#define MAX_AUDIO_FRAME_QUEUE_SIZE 100
#define MAX_VIDEO_FRAME_QUEUE_SIZE 100

#define LOADING_LOWWATER     0

#define LOADING_HIGHWATER     60



enum player_state_e {
    STATE_RUNNED = 0,
    STATE_IDLE,
    STATE_INITED,
    STATE_OPENING,
    STATE_OPENED,
    STATE_EXTRACTOR_READY,
    STATE_EXTRACTOR_STOPPED,
    STATE_EXTRACTOR_EOF,
    STATE_AUDIO_CODEC_READY,
    STATE_AUDIO_CODEC_STOPPED,
    STATE_VIDEO_CODEC_READY,
    STATE_VIDEO_CODEC_STOPPED,
    STATE_PLAYING,
    STATE_STOPPING,
    STATE_STOPPED,
    STATE_AUDIO_PACKET_QUEUE_EMPTY,
    STATE_AUDIO_PACKET_QUEUE_FULL,
    STATE_VIDEO_PACKET_QUEUE_EMPTY,
    STATE_VIDEO_PACKET_QUEUE_FULL,
    STATE_AUDIO_FRAME_QUEUE_EMPTY,
    STATE_AUDIO_FRAME_QUEUE_FULL,
    STATE_VIDEO_FRAME_QUEUE_EMPTY,
    STATE_VIDEO_FRAME_QUEUE_FULL,
    STATE_SEEKING,
    STATE_SEEKED,
    STATE_SWITCH,
    STATE_END,
    STATE_FULLSCREEN,
    STATE_SCREENSIZE_CHANGED,
    STATE_EXIT,
    STATE_CLOSE,
    STATE_LOADING,
    STATE_VIDEO_PACKET_LOADING,
    STATE_AUDIO_PACKET_LOADING
};

struct rect_s {
    int x, y, width, height;
};

struct player_s{
    Obj obj;

    int (*construct)(Player *,char *init_str);
    int (*deconstruct)(Player *);
    int (*set)(Player *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*init)(Player *);
    int (*start)(Player *);
    int (*stop)(Player *);
    int (*exit)(Player *);
    int (*seek_to)(Player *, int position);
    int (*seek_to_specified_pos)(Player *,int64_t position);
    int64_t (*get_duration)(Player *);
    double (*get_position)(Player *);
    int (*get_buffered_position)(Player *);
    int (*get_buffered_duration)(Player *);
    int (*switch_bitrate)(Player *,int bitrate);
    int (*set_media_url)(Player *,char *);
    char *(*get_media_url)(Player *);
    int (*add_subscriber)(Player *, Subscriber *);
    int (*del_subscriber)(Player *, Subscriber *);
    int (*wait_if_true)(Player *player,
                        enum player_state_e conditions[],
                        int condition_cnt,
                        enum player_state_e exceptions[],
                        int exception_cnt,
                        int (*action)(void *arg),
                        void *action_arg,
                        char *who);
    int (*wait_if_false)(Player *player,
                         int (*cond_func)(void *player, enum player_state_e state),
                         enum player_state_e state,
                         char *who);
    int (*wait_if_false_with_except)(Player *player,
                                     int (*cond_func)(void *player, enum player_state_e state),
                                     enum player_state_e state,
                                     enum player_state_e except,
                                     char *who);
    int (*broadcast)(Player *);
    int (*signal)(Player *);
    int (*is_state)(Player *player, enum player_state_e state);
    int (*set_state)(Player *player, enum player_state_e state);
    int (*clear_state)(Player *player, enum player_state_e state);
    char *(*state_to_str)(Player *player, enum player_state_e state);
    int (*add_image)(Player *player, Image *image);
    int (*add_audio)(Player *player, Audio *audio);
    int (*draw_image)(Player *player);
    int (*draw_current_image)(Player *player);
    int (*get_rgb)(Player *player,
                   enum AVPixelFormat fmt,
                   int target_width,
                   int target_height,
                   void *data, int *len);
    int (*get_suitable_rgb_of_screen)(Player *player,
                                      int screen_width,
                                      int screen_height,
                                      enum AVPixelFormat fmt,
                                      int *width,
                                      int *height,
                                      void *data, int *len);
    int (*get_rgb_from_current_frame)(Player *player,
                                      enum AVPixelFormat fmt,
                                      int target_width,
                                      int target_height,
                                      void *data, int *len);
    int (*get_yuv)(Player *player, void *data[], int len[]);
    int (*get_yuv_from_current_frame)(Player *player, void *data[], int len[]);
    int (*publish_message)(Player *player, char *message);

    int (*run)(Player *player);
    int (*open)(Player *player, char *url);
    int (*close)(Player *player);
    int (*print)(Player *player);
    int (*check_loading)(Player *player);

    Centor *message_centor;
    Publisher *publisher;
    Queue *subscriber_queue;

    MediaPeriod *media_period;
    DataSource *data_source;
    Extractor *extractor;
    Codec *video_codec;
    Codec *audio_codec;
    Media_Source *media_source;
    Thread *thread;
    AVSync *sync;
    void *texture;
    String * streamfile;
    int disable_video, disable_audio, disable_subtitle;
    int is_video_valid, is_audio_valid, is_subtitle_valid;

    struct event out_event;
    int out_flag;
    int64_t current_bitrate;
    Image *image;
    Audio *audio;
    int stoped;
    int seeking;
    int switch_flag;
    int64_t seek_timestamp;
    double audio_buffered_time;
    double video_buffered_time;
    Lock * area_lock;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int screen_width;
    int screen_height;

    uint8_t state_runned:1;
    uint8_t state_idle:1;
    uint8_t state_inited:1;
    uint8_t state_opening:1;
    uint8_t state_opened:1;
    uint8_t state_extractor_ready:1;
    uint8_t state_extractor_stopped:1;
    uint8_t state_extractor_eof:1;
    uint8_t state_audio_codec_ready:1;
    uint8_t state_audio_codec_stopped:1;
    uint8_t state_video_codec_ready:1;
    uint8_t state_video_codec_stopped:1;
    uint8_t state_all_ready:1;
    uint8_t state_playing:1;
    uint8_t state_stopping:1;
    uint8_t state_stopped:1;
    uint8_t state_seeking:1;
    uint8_t state_seeked:1;
    uint8_t state_switch:1;
    uint8_t state_end:1;
    uint8_t state_exit:1;
    uint8_t state_audio_packet_queue_empty:1;
    uint8_t state_audio_packet_queue_normal:1;
    uint8_t state_audio_packet_queue_full:1;
    uint8_t state_video_packet_queue_empty:1;
    uint8_t state_video_packet_queue_normal:1;
    uint8_t state_video_packet_queue_full:1;
    uint8_t state_audio_frame_queue_empty:1;
    uint8_t state_audio_frame_queue_normal:1;
    uint8_t state_audio_frame_queue_full:1;
    uint8_t state_video_frame_queue_empty:1;
    uint8_t state_video_frame_queue_normal:1;
    uint8_t state_video_frame_queue_full:1;
    uint8_t state_full_screen:1;
    uint8_t state_screensize_changed:1;
    uint8_t state_close:1;
    uint8_t state_video_packet_loading:1;
    uint8_t state_audio_packet_loading:1;
    uint8_t state_loading:1;

    char url[1024];
    struct rect_s display_rect;
};


int player(int argc, char **argv);


#endif
