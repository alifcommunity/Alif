{ pkgs, lib, config, inputs, ... }:

{
  # https://devenv.sh/basics/
  # env.GREET = "Welcome to Alif development environment!";

  # https://devenv.sh/packages/
  packages = with pkgs; [ 
    gnumake
    gcc
  ];

  # https://devenv.sh/languages/
  languages.cplusplus.enable = true;

  # https://devenv.sh/processes/
  # processes.ping.exec = "ping localhost";

  # https://devenv.sh/services/
  # services.postgres.enable = true;

  # https://devenv.sh/scripts/
  scripts.build-alif.exec = "cd linuxBuild && make";

  enterShell = ''
    echo "مرحباً بك في بيئة تطوير لغة ألف!"
    echo "لبناء المشروع، استخدم الأمر: build-alif"
  '';

  # https://devenv.sh/tests/
  # tests = {
  #   unit-tests = {
  #     exec = "build-alif";
  #   };
  # };

  # https://devenv.sh/pre-commit-hooks/
  # pre-commit.hooks.shellcheck.enable = true;

  # https://devenv.sh/reference/options/
}
