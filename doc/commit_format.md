[update:zip] continue to implement zip.

Description:
The tar.gz which may need compress interface. Inorder to support
multiformat achive, we need to support this requirement.

Major Changes:
1. add squashfs and 7z template.
2. del Gzip which is almost same with GZ test case.
3. add get_file_infos
4. let Achive to suport tar.gz, which need compress firstly.
5. add tar.gz test cases.