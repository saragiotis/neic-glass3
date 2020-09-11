# Expire Message Specification

## Description

Expire is a message generated by the glasscore library. Expire represents a
single detection expiration notification made by glasscore (specifically the
Hypo class), and is used to communicate the detection's expiration to the output
library.  The output library utilizes the Expire message in tracking
and managing detections prior to publication output.

The Expire message uses the [JSON standard](http://www.json.org).

## Output
```json
    {
      "Cmd"  : "Expire",
      "Pid"  : String,
      "Hypo" : { ... }
    }
```

## Glossary

**Values managed by glasscore library**
* Cmd - A string that identifies that this message is an Expire message
* Pid - A string containing an unique identifier for this detection.
* Hypo - An optional object containing the expiring hypo, see
[Hypo](Hypo.md). A Hypo object is only included if it meets glasscore reporting
thresholds.