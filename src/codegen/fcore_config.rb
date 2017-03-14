#! /usr/bin/env ruby

require 'json'
require 'date'

def check_config(cfg)
  abort "no mission name" if not cfg.key? "name"
  abort "no payloads specified" if not cfg.key? "payloads"
  
  payload_ids = []
  
  cfg["payloads"].each do |payload|
    abort "no payload address" if not payload.key? "address"
    abort "no payload name"  if not payload.key? "name"
    abort "no payload priority"  if not payload.key? "priority"
    abort "duplicate payload addresses"  if payload_ids.include? payload["address"]
    abort "invalid payload address" if payload["address"] < 10 or payload["address"] > 127
    payload_ids.push(payload["address"])
  end
end

def write_config(out, config)
  
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
  h.puts ""
  h.puts "extern const FCPayload fcore_payloads[FCORE_PAYLOAD_COUNT];"
  h.close
  
  c.puts "// FCORE Configuration File - #{config["name"]}"
  c.puts "// Generated on #{Date.today} by fcore_config.rb"
  c.puts "\#include \"fcore.h\""
  c.puts ""
  c.puts "const FCPayload fcore_payloads[FCORE_PAYLOAD_COUNT] = {"
  config["payloads"].each do |payload|
    c.puts "    {#{payload["address"]}, \"#{payload["name"]}\", #{payload["priority"]}},"
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
