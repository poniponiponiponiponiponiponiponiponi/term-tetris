{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
  };

  outputs =
  { nixpkgs, self }:
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in
  {
    packages.x86_64-linux.default = pkgs.callPackage ./default.nix {};
    apps.x86_64-linux.client = {
      type = "app";
      program = "${self.packages.x86_64-linux.default}/bin/tetris";
    };
    apps.x86_64-linux.server = {
      type = "app";
      program = "${self.packages.x86_64-linux.default}/bin/tetris-server";
    };
    apps.x86_64-linux.default = self.apps.x86_64-linux.client;
  };
}
