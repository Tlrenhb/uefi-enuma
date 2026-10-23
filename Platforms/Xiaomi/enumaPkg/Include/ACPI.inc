# pipaPkg has NO ACPI.inc: its DSC/Components reference `pipa/AcpiTables.inf`, and the device
# repository's [firmware] packages list points at Silicium-ACPI (Silicium-ACPI/Platforms/Xiaomi).
# This file exists only because the task-13 brief asked for it; it is intentionally without content.
#
# TODO(enuma): decide one of the two, based on what enumaPkg's DSC does with ACPI tables:
#   (a) delete this file (pipa's layout), or
#   (b) if enuma needs its own ACPI table package, put the INF lines here and !include it from enuma.fdf,
#       using enuma's own table files extracted/appended from the stock firmware.
