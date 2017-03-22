#! /usr/bin/env ruby

require 'json'
require 'date'

BAUDS             = 200.0
BYPS              = BAUDS/11.0

PACKET_HEADERSIZE = 12
FRAME_HEADERSIZE  = 4
FRAME_RSSIZE      = 32
FRAME_DATASIZE    = 256 - (FRAME_RSSIZE + FRAME_HEADERSIZE)

BUS_MAXDATA       = 420

def frames(packetBytes)
  packetBytes += PACKET_HEADERSIZE
  return (packetBytes / FRAME_DATASIZE) + 1
end

def bytes(packetBytes)
  return (256 * frames(packetBytes))
end

def calc_core_freq(n1, n2, n3, fs)
  return 0.8 * ( (BYPS - fs) / (n1 + (n2/2.0) + (n3/3.0)) )
end

def check_config(cfg)
  abort "no mission name" if not cfg.key? "name"
  abort "no payloads specified" if not cfg.key? "payloads"
  
  payload_ids = []
  
  n = [0, 0, 0]
  fs = 256/120.0
  
  abort "too many payloads (> 16)" if cfg["payloads"].count > 16
  cfg["payloads"].each do |payload|
    abort "no payload address" if not payload.key? "address"
    abort "no payload name"  if not payload.key? "name"
    abort "no payload priority"  if not payload.key? "priority"
    abort "invalid priority" if payload["priority"] < 1 or payload["priority"] > 3
    abort "duplicate payload addresses"  if payload_ids.include? payload["address"]
    abort "invalid payload address" if payload["address"] < 10 or payload["address"] > 127
    payload_ids.push(payload["address"])
    
    n[payload["priority"] - 1] += 1
  end
end

def write_config(out, config)
  
  n = [0, 0, 0]
  fs = 256/120.0

  config["payloads"].each do |payload|
    n[payload["priority"] - 1] += 1
  end

  core_freq = calc_core_freq(n[0], n[1], n[2], fs)
  
  h = File.open("#{ARGV[1]}.h", "w")
  c = File.open("#{ARGV[1]}.c", "w")
  
  h.puts "// FCORE Configuration File - #{config["name"]}"
  h.puts "// Generated on #{Date.today} by fcore_config.rb"
  h.puts "\#ifndef FCORE_MAIN_INCLUDE"
  h.puts "\#error \"direct inclusion of generated configuration file\""
  h.puts "\#endif"
  h.puts ""
  h.puts "\#define FCORE_MISSION_NAME    \"#{config["name"]}\""
  h.puts "\#define FCORE_CONFIG_GENDATE  \"#{Date.today}\""
  h.puts "\#define FCORE_PAYLOAD_COUNT   (#{config["payloads"].count})"
  h.puts "\#define FCORE_BUS_MAXDATA     (#{BUS_MAXDATA})"
  h.puts "\#define FCORE_BUS_FREQ        (#{core_freq})"
  h.puts "\#define FCORE_BUS_INTERVAL    (#{(bytes(BUS_MAXDATA)/core_freq).ceil})"
  h.puts ""
  h.puts "extern const FCPayload fcore_payloads[FCORE_PAYLOAD_COUNT];"
  h.close
  
  c.puts "// FCORE Configuration File - #{config["name"]}"
  c.puts "// Generated on #{Date.today} by fcore_config.rb"
  c.puts "\#include \"fcore.h\""
  c.puts ""
  c.puts "const FCPayload fcore_payloads[FCORE_PAYLOAD_COUNT] = {"
  config["payloads"].each do |payload|
    c.puts "    {#{payload["address"]}, \"#{payload["name"]}\", #{payload["priority"]}, STATUS_BOOT},"
  end
  c.puts "};"
  c.close
  
end

if ARGV.count != 2 then
  puts "usage: fcore_config infile outfile"
  puts "\tinfile:\tFCORE json configuration file"
  puts "\toutfile:\toutput C header file"
  abort "aborting..."
end

data = File.read(ARGV[0])
config = JSON.parse(data)

check_config config
write_config ARGV[1], config
