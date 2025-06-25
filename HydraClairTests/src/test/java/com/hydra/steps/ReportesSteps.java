package com.hydra.steps;

import io.cucumber.java.After;
import io.cucumber.java.Before;
import io.cucumber.java.en.*;
import io.github.bonigarcia.wdm.WebDriverManager;
import org.openqa.selenium.*;
import org.openqa.selenium.chrome.ChromeDriver;
import org.openqa.selenium.support.ui.*;

import java.time.Duration;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertTrue;

public class ReportesSteps {

    WebDriver driver;
    WebDriverWait wait;

    @Before
    public void setUp() {
        WebDriverManager.chromedriver().setup();
        driver = new ChromeDriver();
        wait = new WebDriverWait(driver, Duration.ofSeconds(10));
        driver.manage().window().maximize();
    }

    @Given("el usuario navega a {string}")
    public void el_usuario_navega_a(String url) {
        driver.get(url);
    }

    @When("selecciona fecha de inicio {string}")
    public void selecciona_fecha_inicio(String fechaInicio) {
        WebElement input = wait.until(ExpectedConditions.elementToBeClickable(By.id("fecha_inicio")));
        input.clear();
        input.sendKeys(fechaInicio);
    }

    @When("selecciona fecha de fin {string}")
    public void selecciona_fecha_fin(String fechaFin) {
        WebElement input = driver.findElement(By.id("fecha_fin"));
        input.clear();
        input.sendKeys(fechaFin);
    }

    @When("elige mostrar {string} registros")
    public void elige_mostrar_registros(String cantidad) {
        WebElement select = driver.findElement(By.id("cantidad"));
        Select dropdown = new Select(select);
        dropdown.selectByValue(cantidad);
    }

    @When("presiona el botón {string}")
    public void presiona_el_boton(String texto) {
        WebElement boton = driver.findElement(By.xpath("//button[contains(text(),'" + texto + "')]"));

        // Asegura que el botón esté en vista
        ((JavascriptExecutor) driver).executeScript("arguments[0].scrollIntoView(true);", boton);
        wait.until(ExpectedConditions.visibilityOf(boton));

        // Clic forzado por JavaScript (omite overlays, footers, etc.)
        ((JavascriptExecutor) driver).executeScript("arguments[0].click();", boton);
    }


    @Then("la tabla debe mostrar hasta {int} filas")
    public void la_tabla_debe_mostrar_hasta_filas(Integer esperado) {
        wait.until(ExpectedConditions.presenceOfElementLocated(By.id("tabla-datos")));
        List<WebElement> filas = driver.findElements(By.cssSelector("#tabla-datos tbody tr"));
        assertTrue(filas.size() <= esperado, "Hay más de " + esperado + " filas.");
    }

    @After
    public void tearDown() {
        if (driver != null) {
            driver.quit();
        }
    }
}
