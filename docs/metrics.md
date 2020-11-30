# Adeptness service metrics availability


```
http://host:port/adeptnessMs/metrics
```

This provides brief information on CPU and memory usage. Example output (formatted
for clarity) is shown here:
```
{
  "Memory":
  {
    "Alloc":329072,
    "TotalAlloc":839680
  },
  "CpuLoadAvg":3.375,
  "CpuTime":0.027213000000000001,
  "CpuAvgUsage":0.0010293528009986004
}
```

The meaning of the fields is as follows:

* `Memory/Alloc` : Amount of heap memory in use, in bytes.
* `Memory/TotalAlloc` : Total heap size, in bytes.
* `CpuLoadAvg` : Average overall CPU usage for the last minute, as a percentage.
* `CpuTime` : The amount of CPU time used by this service, in seconds.
* `CpuAvgUsage`: The amount of CPU time used by this service, as a fraction of elapsed time.

