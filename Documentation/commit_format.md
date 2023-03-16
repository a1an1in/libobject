To make producer reentrant

Description:
inorder to let produce can receive parameter from args, we have to do some
changes at Argument module.

Major Changes:
1.rename Command::run_action to run_command.
2.move add_subcommand position after parse args
3.init producer at app's run_command()