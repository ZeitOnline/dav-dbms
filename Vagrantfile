# -*- mode: ruby -*- vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box_check_update = false

  config.vm.define "default" do |default|
    default.vm.hostname = "modxtransform"

    default.vm.provider "lxc"
    default.vm.box = "fgrehm/trusty64-lxc"

    # dpkg-buildpackage insists on putting the resulting deb in the directory
    # above the sources.
    default.vm.synced_folder ".", "/vagrant/src"

    default.vm.provision "shell", :inline => "export DEBIAN_FRONTEND=noninteractive; echo 'deb http://apt.postgresql.org/pub/repos/apt trusty-pgdg main' >> /etc/apt/sources.list; wget -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -; apt-get update; apt-get -y install devscripts equivs; mk-build-deps -i /vagrant/src/debian/control -t 'apt-get --no-install-recommends -y'"
  end
end
