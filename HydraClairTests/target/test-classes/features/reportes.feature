Feature: Reportes de sensores

  Scenario: Filtrar registros entre fechas válidas
    Given el usuario navega a "http://monitoreohydro.local"
    When selecciona fecha de inicio "2025-06-01"
    And selecciona fecha de fin "2025-06-24"
    And elige mostrar "10" registros
    And presiona el botón "Filtrar"
    Then la tabla debe mostrar hasta 10 filas
