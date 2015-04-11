  <div class="col-md-12">
    <h2>Latest GIFs</h2>

  </div>
  <div class="row">

    <?php foreach (page('gifs')->images() as $image) : ?>
      <div class="col-md-3">
        <a href="<?php echo $image->url() ?>">
          <img src="<?php echo $image->url() ?>" alt="" class="img-responsive">
        </a>
      </div>
    <?php endforeach ?>

  </div>

