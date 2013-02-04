#! /usr/bin/env ruby

require 'spec_helper'
require 'facter/util/ip'

def ifconfig_fixture(filename)
  File.read(fixtures('ifconfig', filename))
end

def netsh_fixture(filename)
  File.read(fixtures('netsh', filename))
end


describe "IPv6 address fact" do
  include FacterSpec::ConfigHelper

  before do
    given_a_configuration_of(:is_windows => false)
  end

  it "should return ipaddress6 information for Darwin" do
    Facter::Util::Resolution.stubs(:exec).with('uname -s').returns('Darwin')
    Facter::Util::IP.stubs(:get_ifconfig).returns("/sbin/ifconfig")
    Facter::Util::IP.stubs(:exec_ifconfig).with(["-a"]).
      returns(ifconfig_fixture('darwin_ifconfig_all_with_multiple_interfaces'))

    Facter.value(:ipaddress6).should == "2610:10:20:209:223:32ff:fed5:ee34"
  end

  it "should return ipaddress6 information for Linux" do
    Facter::Util::Resolution.stubs(:exec).with('uname -s').returns('Linux')
    Facter::Util::IP.stubs(:get_ifconfig).returns("/sbin/ifconfig")
    Facter::Util::IP.stubs(:exec_ifconfig).with(["2>/dev/null"]).
      returns(ifconfig_fixture('linux_ifconfig_all_with_multiple_interfaces'))

    Facter.value(:ipaddress6).should == "2610:10:20:209:212:3fff:febe:2201"
  end

  it "should return ipaddress6 information for Solaris" do
    Facter::Util::Resolution.stubs(:exec).with('uname -s').returns('SunOS')
    Facter::Util::IP.stubs(:get_ifconfig).returns("/usr/sbin/ifconfig")
    Facter::Util::IP.stubs(:exec_ifconfig).with(["-a"]).
      returns(ifconfig_fixture('sunos_ifconfig_all_with_multiple_interfaces'))

    Facter.value(:ipaddress6).should == "2610:10:20:209:203:baff:fe27:a7c"
  end

  it "should return ipaddress6 information for Windows" do
    ENV.stubs(:[]).with('SYSTEMROOT').returns('d:/windows')
    given_a_configuration_of(:is_windows => true)

    fixture = netsh_fixture('windows_netsh_addresses_with_multiple_interfaces')
    Facter::Util::Resolution.stubs(:exec).with('d:/windows/system32/netsh.exe interface ipv6 show address level=verbose').
      returns(fixture)

    Facter.value(:ipaddress6).should == "2001:0:4137:9e76:2087:77a:53ef:7527"
  end
end
