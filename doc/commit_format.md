[update:EventBase] fix bug eventbase bug.

Description:
previously, eventbase seem can't process SIGINT,
it was fixed now.

Major Changes:
1. add set_quit_signal for eventbase.
2. fix test_timer test case.