<?php snippet('header') ?>

  <main class="container">

    <div class="row">
      <h1><?php echo $page->text()->html() ?></h1>
    </div>

    <hr>

    <div class="row">
        <?php snippet('gifs') ?>
    </div>

  </main>

<?php snippet('footer') ?>